setenv BRANCHNN 214
setenv TODAY 2009-09-02     # v214 final
setenv LASTWEEK 2009-08-18     # v213 final
setenv REVIEWDAY 2009-09-09    # preview of v215
setenv LASTREVIEWDAY 2009-08-25  # preview of v214

setenv WEEKLYBLD /cluster/bin/build/scripts
setenv BOX32 titan

setenv CVSROOT /projects/compbio/cvsroot
setenv CVS_RSH ssh

setenv MYSQLINC /usr/include/mysql
if ( "$MACHTYPE" == "x86_64" ) then
    setenv MYSQLLIBS '/usr/lib64/mysql/libmysqlclient.a -lz'
else
    setenv MYSQLLIBS '/usr/lib/mysql/libmysqlclient.a -lz'
endif

setenv USE_SSL 1
setenv USE_BAM 1

if ( "$HOST" == "$BOX32" ) then
    setenv BUILDDIR /scratch/releaseBuild
endif
if ( "$HOST" == "hgwbeta" ) then
    setenv BUILDDIR /data/tmp/releaseBuild
endif
if ( "$HOST" == "hgwdev" ) then
    setenv JAVABUILD /scratch/javaBuild
    setenv JAVA_HOME /usr/java/default
    setenv CLASSPATH .:/usr/share/java:/usr/java/default/jre/lib/rt.jar:/usr/java/default/jre/lib:/usr/share/java/httpunit.jar:/cluster/home/heather/transfer/jtidy.jar:/usr/share/java/rhino.jar:/cluster/home/heather/archive/mysql-connector-java-3.0.16-ga-bin.jar
    # java and ant wont run on hgwdev now without setting max memory
    setenv _JAVA_OPTIONS "-Xmx1024m"
endif

