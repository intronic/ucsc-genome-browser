package edu.ucsc.genome.qa.cgiCheck;
import edu.ucsc.genome.qa.lib.*;
import java.io.*;
import java.net.*;
import java.sql.*;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.Properties;

/** 
 *  For all rows in knownGene, view details page.
 *  Loops over all assemblies.
 *  For all pages viewed, check for non-200 return code.
 *  Doesn't click into any links.
 *  Doesn't check for HGERROR.  
 */
public class HGGeneCheck {

 /**
  * Print usage info and exit
  */
 static void usage() {
    System.out.println(
      "HGGeneCheck - do some basic automatic tests on hgGene cgi\n" +
      "usage:\n" +
      "   java HGGeneCheck propertiesFile\n" +
      "where properties files may contain database, table." +
      "   java HGGeneCheck default\n" +
      "This will use the default properties\n"
      );
    System.exit(1);
 }

 /** 
  *  Runs the program to check all Known Genes,
  *  looping over all assemblies, looking for non-200 return code.
  */
  public static void main(String[] args) {

    boolean debug = false;

    /* Process command line properties, and load them
     * into machine and table. */
    if (args.length != 1)
        usage();
    TestTarget target = new TestTarget(args[0]);

    // make sure CLASSPATH has been set for JDBC driver
    if (!QADBLibrary.checkDriver()) return;
    
    // get read access to database
    HGDBInfo metadbinfo; 
    try {
      metadbinfo = new HGDBInfo("hgwbeta", "hgcentraltest");
    } catch (Exception e) {
      System.out.println(e.toString());
      return;
    }
    if (!metadbinfo.validate()) return;

    ArrayList assemblyList = 
      QADBLibrary.getColumn(metadbinfo, "dbDb", "name", debug);

    // iterate over assembly list
    // System.out.println("iterate over assembly list");
    Iterator assemblyIter = assemblyList.iterator();
    while (assemblyIter.hasNext()) {
      String assembly = (String) assemblyIter.next();
      if (!assembly.equals("mm4")) continue;
      // System.out.println("Assembly = " + assembly);

      // create HGDBInfo for this assembly
      try {
	HGDBInfo dbinfo = new HGDBInfo("localhost", assembly);
	if (!dbinfo.validate()) {
	  System.out.println("Cannot connect to database for " + assembly);
	  continue;
	}
        // does this assembly have knownGene track?
        // could write a helper routine to do this

        // get tracks for this assembly (read track controls from web)
        String hgtracksURL = "http://" + target.machine + "/cgi-bin/hgTracks?db=";
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
          if (!track.equals(target.table)) continue;
          System.out.println(track);
          HgTracks.hggene(dbinfo, target.machine, assembly, track, 
                          target.table, target.quickOn);
          System.out.println();
        }
      } catch (Exception e) {
        System.out.println(e.getMessage());
      }
      System.out.println();
      System.out.println();
    }
  }
}
