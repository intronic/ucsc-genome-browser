#!/hive/groups/encode/dcc/bin/python
import sys, os, re, argparse, subprocess, math, datetime
from ucscgenomics import ra, track

def getMethods(qaDir, args, user, d):
	cmd = "curl -X GET http://genomewiki.cse.ucsc.edu/genecats/index.php/ENCODE_QA 2>/dev/null"
	p = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
	output = p.stdout.read().split("\n")
	r = re.compile('<li class="toclevel.*="tocnumber">(\S+)<.*="toctext">(.*)<\/span>')
	if os.path.exists(qaDir + '/methods'):
		return
	
	f = open(qaDir + '/methods', 'w')
	f.write(args.composite + "\n")
	f.write(args.database + "\n")
	f.write("Release " + args.release + "\n")
	f.write(str(d) + "\n")
	f.write(user + "\n")
	f.write("\n")
	for line in output:
		
		m = r.match(line)
		if m:
			tabs = ""
			for char in m.group(1):
				if char == ".":
					tabs = tabs + "    "
			if tabs == "":
				f.write("\n")
			f.write("%s%s %s\n" % (tabs, m.group(1), m.group(2)))
	return

def writeClaimMail(args, user, qaDir):
	f = open(qaDir + '/claimMail', 'w')
	mail = """Hi Kate,

I am claiming the %s (Release %s) %s track.
Please let me know if there is another track you'd like me to work on instead.

%s
""" % (args.composite, args.release, args.database, user)
	
	f.write(mail)
	f.close()
	return
	
def makeLinks(c, args, qaDir):
	notesfile = c._notesDirectory + "%s.release%s.notes" % (args.composite, args.release)
	downloads = c.downloadsDirectory + "release%s" % args.release
	
	if os.path.exists(qaDir + "/notes.file"):
		os.remove(qaDir + "/notes.file")
	os.symlink(notesfile, qaDir + "/notes.file")
	
	if os.path.exists(qaDir + "/downloads"):
		os.remove(qaDir + "/downloads")
	os.symlink(downloads, qaDir + "/downloads")
	
	return notesfile

def parseNotes(lines):
	tables = set()
	gbdbs = set()
	files = set()
	supplemental = set()
	others = set()
	size = ""
	switch = 0
	p1 = re.compile('New Tables')
	p2 = re.compile('New Download Files')
	p3 = re.compile('New Gbdb Files')
	p4 = re.compile('New Supplemental Files')
	p5 = re.compile('New Other Files')
	s = re.compile('Total: (\d+) MB')
	e = re.compile('^$')
	for i in lines:
		if p1.search(i):
			switch = 1
			continue
		if p2.search(i):
			switch = 2
			continue
		if p3.search(i):
			switch = 3
			continue
		if p4.search(i):
			switch = 4
			continue
		if p5.search(i):
			switch = 5
			continue
		
		sm = s.match(i)
		if sm:
			size = sm.group(1)
			
		if e.match(i):
			switch = 0
			#print "empty line"	
			
		if switch == 1:
			tables.add(i)
		if switch == 2:
			files.add(i)
		if switch == 3:
			gbdbs.add(i)
		if switch == 4:
			supplemental.add(i)
		if switch == 5:
			others.add(i)
			
	return tables, gbdbs, files, supplemental, others, str(size)

def writeTableMail(tables, args, user, qaDir):
	
	if tables:
		sep = ""
		tablestr = sep.join(list(sorted(tables)))
		mail = """Hi Pushers,

For the *%s* database (2 part request):

1) Please push trackDb and friends

2) Please push these %s tables:

%s

from mysqlbeta -> mysqlrr

Reason: releasing ENCODE %s track on %s to the RR http://redmine.soe.ucsc.edu/issues/%s

Thank you!

%s
""" % (args.database, len(tables), tablestr, args.composite, args.database, args.redmine, user)
	else:
		mail = """Hi Pushers,

For the *%s* database:

1) Please push trackDb and friends

from mysqlbeta -> mysqlrr

Reason: releasing _____(fill in metadata or trackDb changes)____ for ENCODE %s track on %s to the RR

Thank you!

%s
""" % (args.database, args.composite, args.database, user)

	f = open(qaDir + "/pushTableMail", "w")
	f.write(mail)
	f.close
	
	f = open(qaDir + "/tableList", "w")
	f.write(tablestr)
	f.close
	
	return tablestr
	
def writeGbdbMail(gbdbs, args, user, qaDir):
	sep = ""
	gbdbstr = sep.join(list(sorted(gbdbs)))
	mail = """
Hi Pushers,

Please push these %s gbdb files:

%s

\tfrom hgwdev --> hgnfs1

Reason: staging %s %s track on beta, http://redmine.soe.ucsc.edu/issues/%s

Thank You!

%s
""" % (len(gbdbs), gbdbstr, args.composite, args.database, args.redmine, user)
	f = open(qaDir + "/pushGbdbsMail", "w")
	f.write(mail)
	f.close()
	return gbdbstr
	
def writeFileMail(files, args, user, qaDir, c):
	sep = ""
	filestr = sep.join(list(sorted(files)))
	mail = """Hi Pushers,

Please push these %s files:

%s

from hgwdev to hgdownload:
%s

Please note the destination on hgdownload is *one directory above the location on dev*

Reason: releasing ENCODE %s on %s to the RR http://redmine.soe.ucsc.edu/issues/%s

Thanks!

%s
""" % (len(files), filestr, c._rrHttpDir, args.composite, args.database, args.redmine, user)

	f = open(qaDir + "/pushFilesMail", "w")
	f.write(mail)
	f.close()
	return filestr

def writeHtml(args, c, qaDir):
	f = open(c._trackDbPath, "r")
	lines = f.readlines()
	f.close
	short = ""
	long = ""
	for i in lines:
		m = re.match('((long|short)Label)\s+(.*)', i)
		
		if m:
			if m.group(1) == 'longLabel':
				long = m.group(3)
			if m.group(1) == 'shortLabel':
				short = m.group(3)
				
	html = """<TR>
        <TD><A HREF="http://hgdownload.cse.ucsc.edu/goldenPath/%s/encodeDCC/%s/"
        TARGET=_BLANK>%s</A></TD>
        <TD>%s</TD>
</TR>""" % (args.database, args.composite, short, long)

	f = open(qaDir + "/htmlDownloadSnippet", "w")
	f.write(html)
	f.close()
	return short, long
	
def runScript(args, qaDir):
	cmd = "qaEncodeTracks.csh %s %s/tableList 2>&1" % (args.database, qaDir)
	p = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, close_fds=True)
	output = p.stdout.read()
	f = open(qaDir + "/script.output", "w")
	sep = ""
	outstr = sep.join(output)
	f.write(outstr)
	f.close()
	return
    
def writeSql(tablestr, filestr, gbdbstr, d, short, long, args, notes, size, user, qaDir):
	newYN = "Y"
	if int(args.release) > 1:
		newYN = "N"
		
	sql = """tbls={
%s
}
files={
%s
}
sizeMB={
%s
}
currLoc={
hgwbeta
}
reviewer={
%s
}
initdate={
%s
}
lastdate={
%s
}
track={
%s
}
releaseLog={
%s
}
priority={
L
}
dbs={
%s
}
newYN={
%s
}
rank={
0
}
ndxYN={
Y
}
joinerYN={
X
}
makeDocYN={
X
}
openIssues={
http://redmine.soe.ucsc.edu/issues/%s
}
bounces={
0
}
notes={
%s
}
releaseLogUrl={
../../cgi-bin/hgTrackUi?db=%s&g=%s
}
pushState={
D
}""" % (tablestr, filestr + "\n" + gbdbstr, size, user, str(d), str(d), short, long, args.database, newYN, args.redmine, notes, args.database, args.composite) 
	f = open(qaDir + "/release.sql", "w")
	f.write(sql)
	f.close()
	return
    
def writePushHtmlMail(args, user, qaDir):

	downloads = 'downloads.html'
	if re.match('mm\S+', args.database):
		downloads = downloadsMouse.html
	
	mail = """Hi Pushers,

Please push the following file:

/usr/local/apache/htdocs/ENCODE/%s

from hgwbeta --> RR.
Reason: added the newly released ENCODE %s %s

Thanks!

%s
""" % (downloads, args.composite, args.database, user)

def main():
	parser = argparse.ArgumentParser(
        prog='qaInit',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description='Initializes QA directory for claiming a release',
        epilog=
"""Example:
qaInir hg19 wgEncodeSydhTfbs 1 69

"""
        )
	parser.add_argument('database', help='The database, typically hg19 or mm9')
	parser.add_argument('composite', help='The composite name, wgEncodeCshlLongRnaSeq for instance')
	parser.add_argument('release', help='The new release to be released')
	parser.add_argument('redmine', help='The Redmins issue number')


	args = parser.parse_args(sys.argv[1:])
	user = os.environ['USER']
	c = track.CompositeTrack(args.database, args.composite)
	qaDir = c.qaInitDir + "release%s" % args.release 
	d = datetime.date.today()
	if not os.path.exists(qaDir):
		os.makedirs(qaDir)
		
	getMethods(qaDir, args, user, d)
	
	makeLinks(c, args, qaDir)
	notesFile = makeLinks(c, args, qaDir)
	writeClaimMail(args, user, qaDir)
	
	m = re.match('.*(kent/.*)', notesFile)
	notes = m.group(1)
	f = open(notesFile, "r")
	lines = f.readlines()
	if not re.match('mkChangeNotes v2', lines[0]):
		print "notes files is not the correct version"
		return
	(tables, gbdbs, files, supplemental, others, size) = parseNotes(lines)
	
	tablestr = writeTableMail(tables, args, user, qaDir)
	gbdbstr = writeGbdbMail(gbdbs, args, user, qaDir)
	filestr = writeFileMail(files | supplemental | others, args, user, qaDir, c)
	(short, long) = writeHtml(args, c, qaDir)
	
	writePushHtmlMail(args, user, qaDir)
	writeSql(tablestr, filestr, gbdbstr, d, short, long, args, notes, size, user, qaDir)
	
	runScript(args, qaDir)



if __name__ == "__main__":
	main()