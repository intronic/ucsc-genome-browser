
import java.io.*;
import java.net.*;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.regex.*;

import com.meterware.httpunit.*;

import org.xml.sax.*;

/**
 * Utilities for getting genome database info 
 *
 */
public class HgTracks {

 /**
  * Zooms out (for now only out) and submits
  *
  * @param page     The active page
  * @param factor   The level of zooming
  */
  // factors: 1 = 1.5x zoom, 2 = 3x zoom, 3 = 10x zoom
  // factor must be 1, 2 or 3
  // use 1 by default
  public static void zoom(WebResponse page, int factor) throws Exception {

    if (factor < 1 || factor > 3) factor = 1;
    String buttonName = "hgt.out" + factor;
    WebForm form = page.getFormWithName("TrackHeaderForm");

    // System.out.println("asking form for submitbutton " + buttonName);
    SubmitButton zoomButton = form.getSubmitButton(buttonName);
    if (zoomButton == null) {
      System.out.println("Error getting zoom button");
      // throw Exception
      return;
    }
    // System.out.println("About to clickzoom button");
    zoomButton.click();
    // System.out.println("Done clicking zoom button");
  }

  /**
   * Refreshes browser screen
   * 
   * @param page     The active page
   */
   public static void refreshHGTracks(WebResponse page) throws Exception {
    // System.out.println("Clicking refresh button");
    WebForm form = page.getFormWithName("TrackForm");
    SubmitButton refreshButton = form.getSubmitButton("submit", "refresh");
    if (refreshButton == null) {
      System.err.println("Error getting refresh button");
      // throw Exception
      return;
    }
    refreshButton.click();
  }

  /**
   * Turns off base-position ruler
   * @param page  The active page
   * @return      True if Base Position has been turned off
   */
   public static boolean turnOffBasePositionHGTracks(WebResponse page) {
    try {
      // System.out.println("Turning off base position");
      WebForm form = page.getFormWithName("TrackForm");
      form.setParameter("ruler", "off");
    } catch (SAXException e) {
      System.err.println(e.getMessage());
      return false;
    }
    return true;
  }

  /**
   * Gets all links on page except track controls
   * Includes Blue Bar links
   *
   * @param page The active page
   */
   public static void printHGTracksImage(WebResponse page) {
  // gets all links except track controls
  // includes blue bar links 
  // (these are not set to true in for-loop
  // because only links from form are checked for "hide")

    try {
      WebForm form = page.getFormWithName("TrackForm");
      // get the links
      WebLink linkarray[] = page.getLinks();

      // review the links
      // screen out track controls
      for (int j = 0; j < linkarray.length; j++) {
        boolean isTrackControl = false;
        String paramarray[] = linkarray[j].getParameterNames();
        for (int l = 0; l < paramarray.length; l++) {
          // get the values for this parameter
          // why make array at all?  why not:
          /*  
            if (form.getParameterValues(paramarray[l].equals("hide")) {
              isTrackControl = true;
              continue;
            }
            */
          String valuearray[] = form.getParameterValues(paramarray[l]);
          for (int m = 0; m < valuearray.length; m++) {
            if (valuearray[m].equals("hide")) {
              isTrackControl = true;
              continue;
            }
          }
        }

        if (!isTrackControl) {
          String text = linkarray[j].asText();
          System.out.println(text);
          // String text2 = linkarray[j].getURLString();
          // System.out.println(text2);
        }
      }
    } catch (Exception e) {}
  }

  /**
   * Checks each record in table to see if hgGene will
   *   give a good page (200 code)
   *
   * @param dbinfo   the host, assembly, user and password object
   * @param machine  the machine on which to run the check
   * @param assembly the genome to check
   * @param table    the table to check
   * @param track    the track to check (same as table)
   */
   public static void hggene(HGDBInfo dbinfo, String machine, String assembly,
                            String track, String table, boolean quickOn) {

    WebConversation wc = new WebConversation();

    int count = 0;
    boolean pb = false;
    ArrayList kglist = QADBLibrary.getGenes(dbinfo, table, pb);

    Iterator kgiter = kglist.iterator();
    while (kgiter.hasNext()) {
      count++;
      if (quickOn && count > 10) return;
      // not using all of the elements
      KnownGene kg = (KnownGene) kgiter.next();
      String name = kg.name;
      String chrom = kg.chrom;
      int txStart = kg.txStart;
      int txEnd = kg.txEnd;
      String url = "http://" + machine + "/cgi-bin/hgGene?db=" + assembly;
      url = url + "&hgg_gene=" + kg.name;
      url = url + "&hgg_chrom=" + kg.chrom;
      url = url + "&hgg_start=" + kg.txStart;
      url = url + "&hgg_end=" + kg.txEnd;
      System.out.println(url);

      WebRequest req = new GetMethodWebRequest(url);

      try {
        WebResponse page = wc.getResponse(req);
        int code = page.getResponseCode();
        if (code != 200) {
          System.out.println("Unexpected response code " + code);
        }
      } catch (Exception e) {
        System.out.println("Unexpected error");
      }
    }
  }

  /**
   * Checks each record in a table to see if browser will
   *   give a good page (200 code).
   * Looks for a specified message string on good pages
   *   (code 200) and saves those.
   * Prints to .err, .msg and .ok files
   *
   * @param dbinfo   the host, assembly, user and password object
   * @param machine  the machine on which to run the check
   * @param assembly the genome to check
   * @param table    the table to check
   */
   public static void hggene(HGDBInfo dbinfo, String machine, String assembly,                            String table) {

    WebConversation wc = new WebConversation();

    boolean pb = false;
    ArrayList kglist = QADBLibrary.getGenes(dbinfo, table, pb);

    Iterator kgiter  = kglist.iterator();

    // open outfiles to separate OK from errors
    String errorFile = new String(assembly + "." + table + ".errors");
    String outFile   = new String(assembly + "." + table + ".ok");
    String msgFile   = new String(assembly + "." + table + ".msg");
    try {
      FileWriter     fwOk   = new FileWriter(outFile);
      BufferedWriter bwOk   = new BufferedWriter(fwOk);
      PrintWriter    outOk  = new PrintWriter(bwOk);

      FileWriter     fwErr  = new FileWriter(errorFile);
      BufferedWriter bwErr  = new BufferedWriter(fwErr);
      PrintWriter    outErr = new PrintWriter(bwErr);

      FileWriter     fwMsg  = new FileWriter(msgFile);
      BufferedWriter bwMsg  = new BufferedWriter(fwMsg);
      PrintWriter    outMsg = new PrintWriter(bwMsg);

      // check each table entry for a returned page
      while (kgiter.hasNext()) {
        // not using all of the elements
        KnownGene kg = (KnownGene) kgiter.next();
        String url = "http://" + machine + "/cgi-bin/hgGene?db=" + assembly +
                     "&hgg_gene="  + kg.name +
                     "&hgg_chrom=" + kg.chrom +
                     "&hgg_start=" + kg.txStart +
                     "&hgg_end="   + kg.txEnd;

        System.out.println(url);

        WebRequest req = new GetMethodWebRequest(url);
        try {
          // System.out.print("trying to get response");
          WebResponse page = wc.getResponse(req);
          int    code = page.getResponseCode();
          String text = page.getText();

          // check for error message
          String msgString = "does not have Exon info";
          Pattern p = Pattern.compile(msgString);
          Matcher m = p.matcher(text);
          boolean b = m.find();

          System.out.println(" code = " + code);
          if (code != 200) {
             // write to errorfile
             outErr.println(url + ":\n Unexpected response code " + code);
             outErr.println("------------------------------------");
          } else if (b == true) {    
             outMsg.println(url + ":\n found: " + msgString);
             outMsg.println("------------------------------------");
          } else {
             outOk.println(url);
          }
        } catch (Exception e) {
          System.out.println("Unexpected error getting response code");
        }
      }
      outOk.close();
      outErr.close();
      outMsg.close();
    } catch (IOException e) {
        System.out.println("Couldn't open output file:" + errorFile);
        System.out.println("   or                     " +   outFile);
    }
  }

  /**
   * Checks each record in table to see if pbTracks (proteome) will
   *   give a good page (200 code).
   * Prints to .err, .ok and .msg  files
   *
   * @param dbinfo   The host, assembly, user and password object
   * @param machine  The machine on which to run the check
   * @param assembly The genome to check
   * @param table    The table to check
   */
   public static void pbgene(HGDBInfo dbinfo, String machine, String assembly,                            String table) {

    WebConversation wc = new WebConversation();

    boolean pb = true;
    ArrayList kglist = QADBLibrary.getGenes(dbinfo, table, pb);

    Iterator kgiter  = kglist.iterator();

    // open outfiles to separate OK from errors
    String errorFile = new String(assembly + ".proteome.errors");
    String outFile   = new String(assembly + ".proteome.ok");
    String msgFile   = new String(assembly + ".proteome.msg");
    String msg2File  = new String(assembly + ".proteome.msg2");
    try {
      FileWriter     fwOk  = new FileWriter(outFile);
      BufferedWriter bwOk  = new BufferedWriter(fwOk);
      PrintWriter    outOk = new PrintWriter(bwOk);

      FileWriter     fwErr  = new FileWriter(errorFile);
      BufferedWriter bwErr  = new BufferedWriter(fwErr);
      PrintWriter    outErr = new PrintWriter(bwErr);

      FileWriter     fwMsg  = new FileWriter(msgFile);
      BufferedWriter bwMsg  = new BufferedWriter(fwMsg);
      PrintWriter    outMsg = new PrintWriter(bwMsg);

      FileWriter     fwMsg2  = new FileWriter(msg2File);
      BufferedWriter bwMsg2  = new BufferedWriter(fwMsg2);
      PrintWriter    outMsg2 = new PrintWriter(bwMsg2);
             
      // check each table entry for a returned page
      int code  = 0;
      int count = 0;
      while (kgiter.hasNext()) {
        // not using all of the elements
        KnownGene kg = (KnownGene) kgiter.next();
        String url = "http://" + machine + "/cgi-bin/pbTracks?db=" + assembly +
                     "&proteinID="  + kg.proteinID;
        //           "&proteinID=HXA7_HUMAN";
        //           "&proteinID=OMD_HUMAN";
        // System.out.println("url = " + url);

        WebRequest req = new GetMethodWebRequest(url);
        try {
          // System.out.print("trying to get response");
          WebResponse page = wc.getResponse(req);
          code = page.getResponseCode();
          String text = page.getText();
          String firstText = text.substring(0, 1000);

          // check for error message
          // String msgString = "does not have";
          String msgString = "Sorry, cannot display";
          Pattern p = Pattern.compile(msgString);
          Matcher m = p.matcher(text);
          boolean b = m.find();
          
          count++;
       
       
          // System.out.println(" code = " + code);
          System.out.println(" URL = " + url);
          // System.out.println(" firstText = " + firstText);
          // System.out.println(" text = " + text);
          System.out.println(" ----------------------------------------------");
          System.out.println(" boolean b = " + b);
          System.out.println(" count = " + count);
          System.out.println(" ----------------------------------------------");
        

          if (code != 200) {
             // write to errorfile
             outErr.println(url + ":\n Unexpected response code " + code);
             outErr.println("------------------------------------");
          } else if (b) {    
             outMsg.println(url + ":\n found: " + msgString);
             outMsg.println("count = " + count);
             outMsg.println("------------------------------------");
             outMsg2.println(kg.proteinID);
             outMsg2.println("count = " + count);
          } else {
             outOk.println(url);
          }
        } catch (Exception e) {
          System.out.println("Unexpected error getting response code");
          outErr.println(url + ":\n Unexpected response code " + code);
          outErr.println(count);
          outErr.println("------------------------------------");
        }
      }
      outOk.close();
      outErr.close();
      outMsg.close();
      outMsg2.close();
    } catch (IOException e) {
        System.out.println("Couldn't open output file:" + errorFile);
        System.out.println("   or                     " +   outFile);
    }
  }

 /**
  *  Steps through all chromosomes to see if zoom-out will work
  *    and give a good page (200 code) for all items in track.
  *  Zooms out 10x twice
  *
  * @param machine     The machine on which to run the check
  * @param assembly    The genome to check
  * @param chroms      The chromosomes to check
  * @param track       The name of the track control
  * @param mode        The chromosome view (all, default) under scrutiny
  * @param defaultPos  The default position for the assembly
  * @param displayMode The view level of the track (hide, squish, etc)
  */
  // track is the name of the track control   
  public static void exerciseTrack(String machine, String assembly,
                            ArrayList chroms, String track, 
                            String mode, String defaultPos,
                            String displayMode) throws Exception {

    String url1, url2, url3;
    PositionIterator mypi;

    // create the WebConversation (httpunit container for session 
    // context)

    WebConversation wc = new WebConversation();

    // System.out.println("Entering exercise track for " + track);

    if (mode.equals("all")) {
      mypi = new PositionIterator(chroms);
    } else if (mode.equals("default")) {
      mypi = new PositionIterator(defaultPos);
    } else {
      System.out.println("mode not supported: " + mode);
      return;
    }

    while (mypi.hasNext()) {

      // System.out.println("Getting next position");
      // currently assuming first 10kb
      // also need to implement middle and last 10kb
      // also full chrom view
      Position mypos = mypi.getNextPosition();

      url1 = "http://" + machine;
      url1 = url1 + "/cgi-bin/hgTracks?db=" + assembly;

      // set position
      String format = mypos.format;
      if (format.equals("full")) {
        // full = position fully defined with start and end coords
        url2 = url1 + "&position=" + mypos.chromName;
        url2 = url2 + ":" + mypos.startPos;
        url2 = url2 + "-" + mypos.endPos;
      } else {
        url2 = url1 + "&position=" + mypos.stringVal;
      }

      // hide all and turn off ruler
      url3 = url2 + "&hgt.hideAll=yes&ruler=off";

      // System.out.println();
      System.out.println(url3);

      WebRequest req = new GetMethodWebRequest(url3);

      WebResponse page = wc.getResponse(req);
      int code = page.getResponseCode();
      if (code != 200) {
        System.out.println("Unexpected response code " + code);
      }
      WebForm form = page.getFormWithName("TrackForm");
      // this will fail gracefully if we attempt an unsupported
      // display mode
      // That is, it returns, throwing an exception
      form.setParameter(track, displayMode);
      HgTracks.refreshHGTracks(page);
      page = wc.getCurrentPage();
      ArrayList links1 = 
        HgTracks.getMatchingLinks(page, "hgc", "DNA");
      HgTracks.checkLinks(wc, links1);
      
      // zoom out 10x
      System.out.println("Calling zoom");
      HgTracks.zoom(page, 3);
      // System.out.println("Get page after zoom");
      page = wc.getCurrentPage();
      // String text = page.getText();
      // int index1 = text.indexOf("size");
      // int index2 = text.indexOf("bp");
      // String size = text.substring(index1, index2);
      // System.out.println(size);
      // System.out.println("Reading for matching links");

      // get and check the links that have hgc in them, but not DNA
      ArrayList links2 = 
        HgTracks.getMatchingLinks(page, "hgc", "DNA");
      // System.out.println("Checking links");
      HgTracks.checkLinks(wc, links2);

      // zoom out 10x again
      System.out.println("Calling zoom again");
      HgTracks.zoom(page, 3);
      // System.out.println("Get page after zoom");
      page = wc.getCurrentPage();
      // System.out.println("Reading for matching links");
      ArrayList links3 = 
        HgTracks.getMatchingLinks(page, "hgc", "DNA");
      // System.out.println("Checking links");
      HgTracks.checkLinks(wc, links3);
    }
  }

 /**
  * Check first lines of WebLink objects,
  *   and report if response code is other than 200
  *   or report if "HGERROR"
  *
  * @param wc          The open web conversation
  * @param links       An ArrayList of links to check
  */
  // given a ArrayList of WebLink objects,
  // follow up to MAXLINKS of them
  // report if response code is other than 200
  // report if "HGERROR"
  public static void checkLinks(WebConversation wc, ArrayList links) 
                         throws Exception {
    
    final int MAXLINKS = 4;
    int count = 0;
    Iterator iter = links.iterator();
    while (iter.hasNext()) {
      WebLink link = (WebLink) iter.next();
      link.click();
      WebResponse page = wc.getCurrentPage();
      int code = page.getResponseCode();
      if (code != 200) {
        System.out.println("Unexpected response code " + code);
      }
      String text = page.getText();
      int index = text.indexOf("HGERROR");
      if (index > 0) {
        System.out.println("Error detected:");
        String errortext = text.substring(index);
        System.out.println(errortext);
      }
      count++;
      if (count > MAXLINKS) return;
    }
  }

 /**
  * Creates an ArrayList of WebLink objects
  * with the property that matchString is in the URL
  * and excludeString is not the ALT tag
  *
  * @param page           The active page
  * @param matchString    The string that must be in the URL
  * @param excludeString  The string that must not be in the URL
  * @return               List of links that meet the criteria
  */
  // creates an ArrayList of WebLink objects
  // with the property that matchString is in the URL
  // and excludeString is not the ALT tag
  public static ArrayList getMatchingLinks(WebResponse page, 
                                    String matchString,
                                    String excludeString) throws Exception {

    ArrayList matchLinks = new ArrayList();
    int count = 0;

    // System.out.println("Get matching links");

    WebLink links[] = page.getLinks();
    for (int i=0; i < links.length; i++) {
      String url = links[i].getURLString();
      int index = url.indexOf(matchString);
      String text = links[i].asText();
      // System.out.println("linktext = " + text);
      if (index != -1 && !text.equals(excludeString)) {
        matchLinks.add(links[i]);
        count++;
        // System.out.println(links[i].asText());
      }
    }
    System.out.println("Links found = " + count);
    return (matchLinks);
  }

 /**
  * Makes ArrayList of all tracks in the assembly name in myURL
  *
  * @param myURL       The URL of the query, contains assembly name
  * @param defaultPos  The default position of the assembly
  * @param debug       Whether debug mode is on
  * @return            List of tracks in currrent assembly
  */ 
  public static ArrayList getTrackControls(String myURL, String defaultPos, 
                                          boolean debug) {

    ArrayList paramlist = new ArrayList();

    WebConversation wc = new WebConversation();

    myURL = myURL + "&position=" + defaultPos;
    myURL = myURL + "&hgt.hideAll=yes";

    if (debug) System.out.println(myURL);
    WebRequest req = new GetMethodWebRequest(myURL);

    try {
      WebResponse page = wc.getResponse(req);
      int code = page.getResponseCode();
      if (code != 200) {
        System.out.println("Unexpected response code = " + code);
      }
      page = wc.getCurrentPage();
      WebForm form = page.getFormWithName("TrackForm");
      String paramarray[] = form.getParameterNames();
      for (int i=0; i < paramarray.length; i++) {
        if (debug) System.out.println(paramarray[i]);
        // get the values for this parameter
        String valuearray[] = form.getParameterValues(paramarray[i]);
        for (int j=0; j < valuearray.length; j++) {
          // check if hide is value
          if (valuearray[j].equals("hide")) {
            paramlist.add(paramarray[i]);
            break;  // question:  break how far back?  just local for-loop?
          }
        }
      }
    } catch (Exception e) {
      System.out.println(e.getMessage());
    }
    return (paramlist);
  }

 /**
  * Sets all track controls to "hide"
  *
  * @param page  The active page
  * @return      True if successful
  */
  public static boolean hideAllHGTracks(WebResponse page) {

    try {
      // only 1 form on hgTracks
      WebForm form = page.getFormWithName("TrackHeaderForm");
      SubmitButton hideButton = form.getSubmitButton("hgt.hideAll");
      if (hideButton == null) {
        System.err.println("Error getting hide button");
	return false;
      }
      // System.out.println("Clicking hide all");
      hideButton.click();
    } catch (MalformedURLException e) {
      System.err.println(e.getMessage());
      return false;
    } catch (SAXException e) {
      System.err.println(e.getMessage());
      return false;
    } catch (IOException e) {
      System.err.println(e.getMessage());
      return false;
    }
    return true;
  }

 /**
  * Prints all links to command line
  *
  * @param page  The active page
  */
  public static void printLinks(WebResponse page) {
    try {
      WebLink linkarray[] = page.getLinks();
      System.out.println("List of links:");
      for (int i=0; i< linkarray.length; i++) {
        System.out.println(linkarray[i].asText());
        // System.out.println(linkarray[i].getURLString());
      }
    } catch (SAXException e) {
      System.err.println(e.getMessage());
    }
  }

 /**
  * Checks if link is http or https and clicks it.
  * Prints URL or "skipping" to command line.
  *
  * @param link  The link being checked
  * @return      True if is correctly formed, regardless of type 
  */
  public static boolean checkLink(WebLink link) {
    try {
      System.out.println("Link = " + link.asText());
      WebRequest detailReq = link.getRequest();
      URL myURL = detailReq.getURL();
      String myProtocol = myURL.getProtocol();
      if (myProtocol.equals("http") | myProtocol.equals("https")) {
        System.out.println("URL = " + myURL.toString() + "\n");
        link.click();
      } else {
        System.out.println("Skipping protocol " + myProtocol);
      }
    } catch (Exception e) {
      System.err.println(e.getMessage());
      return false;
    }
    return true;
  }

 /**
  * Clicks link and gets response code from server
  *
  * @param wc    The open web connection
  * @param link  The link
  * @return      The response code    
  */
  // link is known to be URL
  public static int getResponseCode(WebConversation wc, WebLink link) {
    try {
        link.click();
	WebResponse resp = wc.getCurrentPage();
	return resp.getResponseCode();
    } catch (Exception e) {
        System.err.println(e.getMessage());
        return 0;
    }
  }

 /**
  * Gets list of links from image in browser
  *
  * @param trackContainer  The track display properties: position coords
  * @param baseURL         The http, machine name and assembly info
  * @param skiplinks       The number of initial links not to check
  * @param tc1             Text that prevents a link from being added to list 
  * @param debug           Whether debug mode is on
  * @return                The list of links
  */
  public static ArrayList getListFromImage(TrackContainer trackContainer,
                                    String baseURL,
                                    int skiplinks, String tc1,
                                    boolean debug) {

    ArrayList weblist = new ArrayList();

    if (debug) {
      System.out.println("Web values");
    }

    WebConversation wc = new WebConversation();

    String webURL = baseURL + "&position=";
    webURL = webURL + trackContainer.chrom + ":";
    webURL = webURL + trackContainer.startpos + "-";
    webURL = webURL + trackContainer.endpos;

    String hideURL = webURL + "&hgt.hideAll=yes";

    WebRequest req = new GetMethodWebRequest(hideURL);

    try {

      // hide all
      WebResponse page = wc.getResponse(req);

      // get the page again
      page = wc.getCurrentPage();

      // req = new GetMethodWebRequest(webURL);

      // page = wc.getResponse(req);

      // switch to "hide all" mode
      // HgTracks.hideAllHGTracks(page);
    
      // get the page again
      // page = wc.getCurrentPage();

      // turn off base position track control 
      HgTracks.turnOffBasePositionHGTracks(page);
      HgTracks.refreshHGTracks(page);

      // get the page again
      page = wc.getCurrentPage();

      WebForm form = page.getFormWithName("TrackForm");
      String paramarray[] = form.getParameterNames();

      // find the track control
      // could I assume that it is there?
      for (int i = 0; i < paramarray.length; i++) {
        if(paramarray[i].equals(trackContainer.trackcontrol)) {
          form.setParameter(trackContainer.trackcontrol, "full");
          HgTracks.refreshHGTracks(page);
          // get the page again
          page = wc.getCurrentPage();
          // get the links
          WebLink linkarray[] = page.getLinks();
          // print the links
          // skip past skiplinks, and past the track control
          for (int j = skiplinks + 2; j < linkarray.length; j++) {
            String text = linkarray[j].asText();
            if (text.equals(tc1)) break;
            if (debug) {
              System.out.println(text);
            }
            weblist.add(text);
          }
          break;
        } // end if
      } // end for
    
    } catch (Exception e) {
      System.err.println(e.getMessage());
    }
    return (weblist);
  }
}
