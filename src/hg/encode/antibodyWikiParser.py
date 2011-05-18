#!/usr/bin/env python

#
# antibodyWikiParser.py: parse any to-be-registered antibodies from the wiki,
# and download any newly-approved validation documents.
#

import base64
from BeautifulSoup import BeautifulSoup
import HTMLParser
from optparse import OptionParser
import re
import sys
import urllib2 
from ucscgenomics.rafile.RaFile import *



def stripLeadingTrailingWhitespace(text):
    """Given a string, remove any leading or trailing whitespace"""
    text = re.sub("^( )+", "", text)
    text = re.sub("( )+$", "", text)
    return(text)

def getContents(field):
    """Given an HTML field, return the contents"""
    return(stripLeadingTrailingWhitespace(field.contents[0]))

def processSource(orderEntry):
    """ 
    Given the contents of the 'Source' column, parse out the vendor name, vendor ID,
    and order URL if any.  We are assuming that from the textual contents of the 
    cell (which excludes the order URL), the last word will be the vendor ID and the
    preceding words will make up the vendor name.  Return a list containing, in order,
    vendorName, vendorId, and orderUrl.
    """
    urlClauses = orderEntry.findAll("a")
    if len(urlClauses) == 0:
        orderUrl = ""
        orderInfo = getContents(orderEntry)
    else:
        orderUrl = urlClauses[0]["href"]
        orderInfo = getContents(urlClauses[0])
    if len(orderInfo) == 0:
        vendorId = ""
        vendorName = ""
    else:
        orderData = orderInfo.split()
        if len(orderData) > 0:
            vendorId = orderData[-1]
            vendorName = ' '.join(orderData[0:-1])
        else:
            vendorId = ""
            vendorName = ""
    return((vendorName, vendorId, orderUrl))


def processFactorId(factorEntry):
    """
    Given the contents of the 'Factor ID' column, parse out the factor name (targetID)
    and the target URL.  We assume that the target URL points ingo GeneCards, and 
    that the factor name and or URL might or might not be specified.  Return a list 
    containing the target ID and targetUrl.
    """
    urlClauses = factorEntry.findAll("a")
    targetId = getContents(factorEntry)
    if len(urlClauses) > 0:
        #
        # A URL was provided.  If no target ID was provided, pull one out
        # of the URL.
        targetUrl = urlClauses[0]["href"]
        if len(targetId) == 0:
            tokens = re.split("gene=", targetUrl)            
            targetId = tokens[1]
    else:
        #
        # No URL was provided.  Probably the contents of this cell list
        # a locus name (hopefully with HUGO standards, one can dream...)
        # so we can assemble a putative target URL from that locus name.
        locusName = getContents(factorEntry)
        targetId = "GeneCards:" + locusName
        targetUrl = "http://www.genecards.org/cgi-bin/carddisp.pl?gene=" + locusName
    return((targetId, targetUrl))



def processValidation(validationCell, species, antibody, lab, downloadsDir,
                      username, password, wikiBaseUrl):
    """
    (1) Given the contents of the 'Validation' cell, plus the species, antibody, and
    lab, assemble a validation filename that fits the naming convention.
    (2) If there's any evidence of a filename, set the validation string to it.  
    Otherwise, set the validation string to "missing".
    (3) If there is a hyperlink to a validation file, download it into the 
    filename assembled here, in the indicated download directory.
    Assumptions:
    - users are illogical
    - the number of ways users can do unexpected things cannot be counted
    """
    validationCellContents = getContents(validationCell)
    if validationCellContents is "missing" or len(validationCell.findAll("a")) == 0:
        validation = "missing"
    else:
        #
        # In the off-chance that the user has specified the validation type as the label
        # in the wiki table, where the filename is linked, pull out the validation type.
        # Assemble a validation label string for the CGIs, containing the antibody name,
        # and validation type in parentheses.
        # If the user has specified the antibody name in the label, it will need to be stripped
        # out to ensure that it's not included twice.  Parentheses complicate things.
        # At the writing of this script (May 3, 2011), the naming convention for antibodies
        # is <targetName>_(<vendorId>), such as TRIM37_(SC-49548).  When stripping out
        # the antibody name, be sure to escape any parentheses, so that they can be processed
        # as literals in the regular expression clause.
        #
        validationType = validationCellContents
        antibodyPieces = re.split("[\(\)]", antibody)
        searchReplaceString = antibodyPieces[0]
        if len(antibodyPieces) > 1:
            searchReplaceString = searchReplaceString + "\(" \
                + antibodyPieces[1] + "\)"
        validationType = re.sub(searchReplaceString, "", validationType)
        #
        # Remove any extraneous parentheses, any any leading or trailing whitespace
        validationType = re.sub("[\(\)]", "", validationType)
        validationType = re.sub("^( )+", "", re.sub("( )+$", "", validationType))
        validationLabel = antibody + "(" + validationType + ")"

        #
        # Here, download the validation document, if provided, into the expected filename.
        # When generating the document name, strip any parentheses
        # from the antibody name.  Parentheses in filenames are just bad...
        documentName = species + "_" + re.sub("[\(\)]", "", antibody) \
                       + "_validation_" + lab + ".pdf"
        #
        # If a hyperlink exists, download the document to the downloads directory
        urlClauses = validationCell.findAll("a")
        if len(urlClauses) > 0:
            if urlClauses[0].has_key("href"):
                url = urlClauses[0]["href"]
                # 
                # If this document is not in PDF format (e.g. if it's a word doc, 
                # print out a warning message and don't try to download it, but return the
                # target filename (generated above).  If the document is a PDF,
                # download it and save it in the filename generated above.
                if re.search(".pdf$", url):
                    validationData = accessWiki(wikiBaseUrl + url, username, password)
                    if len(validationData) > 0:
                        newValidationFile = open(downloadsDir + "/" + documentName, "wb")
                        newValidationFile.write(validationData)
                        newValidationFile.close()
                    else:
                        print "Warning: not downloading", url, ": not a PDF file";
                    validation =  validationLabel + ":" + documentName
                    validation = re.sub("(\s)+", "", validation)
    return(validation)


    
#
# Process an antibody table entry, in which the order of the columns is
# (Antibody, AntibodyDescription, TargetDescription, Source, Lab, Lot(s), 
#  FactorId, ValidationDocument)
#   
def processAntibodyEntry(entry, species, downloadsDirectory, username, password,
                         wikiBaseUrl):
    """
    For a single wiki table entry, generate an appropriate RA file stanza and
    download any validation documents into the download directory, into a filename
    that meets the naming convention for antibody documents.  When finished, print
    a new stanza to stdout.
    """
    cells = entry.findAll("td")
    #
    # Skip over any example entries
    if re.search("(Example)", getContents(cells[0])):
        return((None, False))
    else: 
        stanza = RaStanza()
        term = getContents(cells[0])
        (vendorName, vendorId, orderUrl) = processSource(cells[3])
        #
        # The naming standard (as of May 3, 2011) is to name antibodies as 
        # <target>_(<vendorId>), such as TAF7_(SC-101167).  In the "term" cell,
        # the antibody might already have that name, or (more likely) it might be 
        # named by just the target.  If the vendor ID isn't in the name yet, add it. 
        if re.search(vendorId, term):
            stanza["term"] = term
        else:
            stanza["term"] = term + "_(" + vendorId + ")"
        stanza["tag"] = re.sub("[-_\(\)]", "", stanza["term"])
        stanza["type"] = "Antibody"
        stanza["antibodyDescription"] = getContents(cells[1])
        stanza["target"] = re.split("_", stanza["term"])[0]
        stanza["targetDescription"] = getContents(cells[2])
        stanza["vendorName"] = vendorName 
        stanza["vendorId"] = vendorId 
        stanza["orderUrl"] = orderUrl
        stanza["lab"] = getContents(cells[4])
        stanza["lots"] = getContents(cells[5])
        (stanza["targetId"], 
         stanza["targetUrl"]) = processFactorId(cells[6])
        stanza["validation"] = processValidation(cells[7], species, stanza["term"], 
                                                 stanza["lab"], downloadsDirectory, 
                                                 username, password, wikiBaseUrl)
        #
        # Indicate whether or not the document (if any) is approved by the NHGRI
        if re.search("^[Y|y]", getContents(cells[8])):
            approved = True
        else:
            approved = False
        return((stanza, approved))
                               

def accessWiki(url, username, password):
    """Read the indicated URL from the wiki page"""
    passmgr = urllib2.HTTPPasswordMgrWithDefaultRealm()
    base64string = base64.encodestring('%s:%s' % (username, password))[:-1]
    authheader =  "Basic %s" % base64string
    req = urllib2.Request(url)
    req.add_header("Authorization", authheader)
    try:
        handle=urllib2.urlopen(req)
        return(handle.read())
    except IOError, e:
        print "Fail!  Bad username or password?"
        return(None)



#
# Main code
#
wikiBaseUrl = "http://encodewiki.ucsc.edu/"
defaultUsername = "encode"
defaultPassword = "human"
parser = OptionParser()
parser.add_option("-s", "--species", dest="species", default="human",
                  help="Species")
parser.add_option("-d", "--downloadDir", dest="downloadDirectory", default=".",
                  help="Directory to download any validation documents into")
parser.add_option("-u", "--username", dest="username", default=defaultUsername,
                  help="Username to access the wiki page")
parser.add_option("-p", "--password", dest="password", default=defaultPassword,
                  help="Password to access the wiki page")
parser.add_option("-f", "--force", dest="forcePrinting", default=False,
                  help="Force printing of all stanzas, whether or not there's NHGRI approval")
(parameters, args) = parser.parse_args()

#
# Read the antibodies page.  If successful, proceed to the new antibodies table,
# which is the second table on the page.  Each row in the table, after the header
# row, is either an example or a new antibody registration.  So, process each row
# after the first.
#
antibodiesPage = accessWiki(wikiBaseUrl+"EncodeDCC/index.php/Antibodies", 
                            parameters.username, parameters.password)
if antibodiesPage != None:
    soup = BeautifulSoup(antibodiesPage)
    antibodyEntryTable = soup.findAll("table")[1]
    skippedHeaderRow = False
    for entry in antibodyEntryTable.findAll("tr"):
        if not skippedHeaderRow:
            skippedHeaderRow = True
        else:
            (newStanza, approvedByNhgri) = processAntibodyEntry(entry, parameters.species, 
                                                                parameters.downloadDirectory,
                                                                parameters.username,
                                                                parameters.password, wikiBaseUrl)
            if approvedByNhgri or parameters.forcePrinting:
                if newStanza is not None:
                    print newStanza
