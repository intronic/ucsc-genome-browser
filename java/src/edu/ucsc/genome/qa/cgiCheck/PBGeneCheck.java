package edu.ucsc.genome.qa.cgiCheck;
import edu.ucsc.genome.qa.lib.*;
import java.io.*;
import java.net.*;
import java.sql.*;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.Properties;


 // Heather Trumbower & Robert Kuhn
 // Modified from SGDGeneCheck.java
 // 01-09-04

/** 
 *  Works out Proteome Browser.
 *  For all rows in knownGene, view details page.
 *  For all pages viewed, check for non-200 return code.
 *  Doesn't click into any links.
 *  Doesn't check for HGERROR.  
 *  Prints to .ok, .error files via HgTracks.pbgene.
 */
public class PBGeneCheck {

 /** 
  * Checks all Known Genes for successful links in Proteome Browser.
  *
  */
  public static void main(String[] args) {

    boolean debug = false;

    String machine  = "hgwdev.cse.ucsc.edu";
    String assembly = "hg16";
    String table    = "knownGene";
    String errMsg   = "Sorry, cannot display";  // not used yet.  
                                         // want to pass to QAWebLibraray.pbgene

    // make sure CLASSPATH has been set for JDBC driver
    if (!QADBLibrary.checkDriver()) return;
    
    // get read access to database
    HGDBInfo metadbinfo, dbinfo; 
    try {
      metadbinfo = new HGDBInfo("hgwbeta", "hgcentraltest");
      dbinfo = new HGDBInfo("localhost", assembly);
    } catch (Exception e) {
      System.out.println(e.toString());
      return;
    }
    if (!metadbinfo.validate()) return;

    if (!dbinfo.validate()) {
      System.out.println("Cannot connect to database for " + assembly);
      return;
    }

    HgTracks.pbgene(dbinfo, machine, assembly, table);

/*
    try {
      // get tracks for this assembly (read track controls from web)
      String hgtracksURL = "http://" + machine + "/cgi-bin/hgTracks?db=";
      hgtracksURL = hgtracksURL + assembly;
      String defaultPos = QADBLibrary.getDefaultPosition(metadbinfo,assembly);
      ArrayList trackList = 
        HgTracks.getTrackControls(hgtracksURL, defaultPos, debug);
      if (debug) {
        int count2 = trackList.size();
        System.out.println("Count of tracks found = " + count2);
      }
      // iterate through tracks
      Iterator trackIter = trackList.iterator();
      while (trackIter.hasNext()) {
        String track = (String) trackIter.next();
        if (debug) {
          System.out.println("track = " + track);
          System.out.println("table = " + table + "\n");
        }
        if (!track.equals(table)) continue;
        System.out.println();
      }
    } catch (Exception e) {
      System.out.println(e.getMessage());
    }
 */


  }
}
