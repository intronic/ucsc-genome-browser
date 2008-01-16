#!/usr/bin/ruby -w

# encodeLoad.rb - load ENCODE data submission generated by the
#                       automated submission pipeline
# Reads load.ra for information about what to do

# Writes error or log information to STDOUT
# Returns 0 if load succeeds.

# DO NOT EDIT the /cluster/bin/scripts copy of this file -- 
# edit the CVS'ed source at:
# $Header: /projects/compbio/cvsroot/kent/src/hg/encode/encodeLoad/doEncodeLoad.rb,v 1.5 2008/01/16 03:07:26 galt Exp $

# Global constants

# Global variables
$opt_verbose = 1
$loadRa = 'load.ra'
$submitDir = ""
$submitType = ""
$tempDir = "/data/tmp"
$encodeDb = "hg18"

#probably get rid of this:
$err = false

def errAbort(msg)
  STDERR.puts msg
  exit 1
end

def usage 
    errAbort "usage: doEncodeLoad.rb submission_type project_submission_dir\n"
end

def verbose(level, string)
  if level <= $opt_verbose
    STDERR.puts string
  end
end



def loadWig(tableName, fileList)
  if system( "head -1000 -q #{fileList} | wigEncode stdin stdout #{tableName}.wib | hgLoadWiggle -pathPrefix=/gbdb/#{$encodeDb}/wib -tmpDir=#{$tempDir} #{$encodeDb} #{tableName} stdin >loadWig.out 2>&1" )
      system( "rm -f /gbdb/#{$encodeDb}/wib/#{tableName}.wib" )
      system( "ln -s #{tableName}.wib /gbdb/#{$encodeDb}/wib" )
      print "#{fileList} Passed\n";
  else 
      STDERR.print "ERROR: File(s) '#{fileList}' failed wiggle validation.\n";
      errAbort File.read("validateWig.out")
  end
end

def loadBed3(tableName, fileList)

  system "echo $SHELL" #debug
  puts  "head -1000 -q #{fileList} | egrep -v '^track|browser' | hgLoadBed #{$encodeDb} #{tableName} stdin >loadBed.out 2>&1"  #debug

  if system( "head -1000 -q #{fileList} | egrep -v '^track|browser' | hgLoadBed #{$encodeDb} #{tableName} stdin >loadBed.out 2>&1")
      print "#{fileList} Passed\n";
      #debug restore: File.delete "bed.tab"
  else
      STDERR.print "ERROR: File(s) '#{fileList}' failed bed load.\n";
      errAbort File.read("loadBed.out")
  end
end

############################################################################
# Misc subroutines

def readRaFile(file)
  inRecord = false
  ra = []
  hash = {}
  keyword = nil
  f = File.open(file)
  f.readlines.each do |line|
    line.chomp!
    STDERR.puts "#{line}\n"  #debug
    if line[0,1] == '#'
      continue
    end
    if line == ""
      if inRecord
        ra << [keyword,hash]
        hash = {}
      end
      inRecord = false
    else
      spc = line.index(' ')
      if spc == nil
        errAbort "syntax error: invalid line in .ra file: #{line}\n"
      end
      word = line[0,spc]
      value = line[spc+1,line.length]
      #STDERR.puts "debug: #{word}=#{value}\n"  #debug
      unless inRecord
        inRecord = true
        keyword = value
      end
      hash[word] = value
    end
  end
  if inRecord
    ra << [keyword,hash]
  end
  f.close
  return ra
end


############################################################################
# Main

# Change dir to submission directory obtained from command-line

if ARGV.length != 2
  usage
end


$submitType = ARGV[0]	# currently not used
$submitDir = ARGV[1]

verbose 1, "Loading submission in directory #{$submitDir}\n"
Dir.chdir $submitDir


# Load files listed in load.ra

ra = readRaFile $loadRa

#STDERR.puts "debug: ra.length=#{ra.length}\n"  #debug
#STDERR.puts "debug: #{ra.inspect}\n"  #debug

ra.each do |x|
  h = x[1]
  STDERR.puts "debug: #{x[0]}\n"  #debug
  STDERR.puts "  tablename #{h["tablename"]}\n"  #debug
  STDERR.puts "  type      #{h["type"]}\n"  #debug
  STDERR.puts "  assembly  #{h["assembly"]}\n"  #debug
  STDERR.puts "  files     #{h["files"]}\n"  #debug
  case h["type"]
  when "wig"
    STDERR.puts "  it's a WIGGLE!\n"  #debug
    loadWig h["tablename"], h["files"]
  when "bed3"
    STDERR.puts "  it's a BED3!\n"  #debug
    loadBed3 h["tablename"], h["files"]
  else
    errAbort "unexpected error: unknown type #{h["type"]} in load.ra\n"
  end 
end

exit 0

