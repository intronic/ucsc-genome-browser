char formQ[] =
"\n"
"<h1> Push Queue Form\n"
"\n"
"<form ACTION=\"/cgi-bin/qaPushQ\" NAME=\"mainForm\" METHOD=\"POST\">\n"
"\n"
"\n"
"<input TYPE=\"hidden\" NAME=\"action\" VALUE=\"post\"  >\n"
"<input TYPE=\"hidden\" NAME=\"qid\"    VALUE=\"<!qid>\">\n"
"\n"
"<table cellpadding=6>\n"
"\n"
"<tr><td align=\"right\">\n"
"Priority:\n"
"</td>\n"
"<td>\n"
"<select NAME=\"priority\" SIZE=1>\n"
"<option <!sel-priority-A> value=\"A\">A</option>\n"
"<option <!sel-priority-B> value=\"B\">B</option>\n"
"<option <!sel-priority-C> value=\"C\">C</option>\n"
"<option <!sel-priority-D> value=\"D\">D</option>\n"
"<option <!sel-priority-L> value=\"L\">L</option>\n"
"</select>\n"
"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Queue Id: <!qid> \n"
"&nbsp;&nbsp;<a href=\"/cgi-bin/qaPushQ\">CANCEL</a>\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"Date Opened:\n"
"</td>\n"
"<td>\n"
"<input TYPE=\"text\" NAME=\"qadate\" SIZE=\"10\" VALUE=\"<!qadate>\">\n"
"&nbsp;&nbsp;&nbsp;&nbsp;Initial submission: <!initdate>\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"New track?:\n"
"</td>\n"
"<td>\n"
"<select NAME=\"newYN\" SIZE=1>\n"
"<option <!sel-newYN-N> value=\"N\">N</option>\n"
"<option <!sel-newYN-Y> value=\"Y\">Y</option>\n"
"</select>\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"Track:\n"
"</td>\n"
"<td>\n"
"<TEXTAREA NAME=\"track\", ROWS=\"4\", COLS=\"40\"><!track></TEXTAREA>\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"Databases:\n"
"</td>\n"
"<td>\n"
"<input TYPE=\"text\" NAME=\"dbs\" SIZE=\"40\" VALUE=\"<!dbs>\">\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"Tables:\n"
"</td>\n"
"<td>\n"
"<TEXTAREA NAME=\"tbls\", ROWS=\"5\", COLS=\"40\"><!tbls></TEXTAREA>\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"CGIs:\n"
"</td>\n"
"<td>\n"
"<TEXTAREA NAME=\"cgis\", ROWS=\"5\", COLS=\"40\"><!cgis></TEXTAREA>\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"Files:\n"
"</td>\n"
"<td>\n"
"<TEXTAREA NAME=\"files\", ROWS=\"5\", COLS=\"40\"><!files></TEXTAREA>\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"Size(MB):\n"
"</td>\n"
"<td>\n"
"<input TYPE=\"text\" NAME=\"sizeMB\" SIZE=\"40\" VALUE=\"<!sizeMB>\">\n"
"&nbsp;\n"
"&nbsp;\n"
"&nbsp;\n"
"&nbsp;\n"
"&nbsp;\n"
"<input TYPE=SUBMIT NAME=\"showSizes\" VALUE=\"Show Sizes\">\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"Current Location:\n"
"</td><td>\n"
"<select NAME=\"currLoc\" SIZE=1>\n"
"<option <!sel-currLoc-hgwdev> value=\"hgwdev\">hgwdev</option>\n"
"<option <!sel-currLoc-hgwbeta> value=\"hgwbeta\">hgwbeta</option>\n"
"</select>\n"
"\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"Makedoc verified?: </td>\n"
"<td>\n"
"<select NAME=\"makeDocYN\" SIZE=1>\n"
"<option <!sel-makeDocYN-N> value=\"N\">N</option>\n"
"<option <!sel-makeDocYN-Y> value=\"Y\">Y</option>\n"
"</select>\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"Online Help:\n"
"</td>\n"
"<td>\n"
"<input TYPE=\"text\" NAME=\"onlineHelp\" SIZE=\"40\" VALUE=\"<!onlineHelp>\">\n"
"</select>\n"
"\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"Index verified?: </td>\n"
"<td>\n"
"<select NAME=\"ndxYN\" SIZE=1>\n"
"<option <!sel-ndxYN-N> value=\"N\">N</option>\n"
"<option <!sel-ndxYN-Y> value=\"Y\">Y</option>\n"
"</select>\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"All.Joiner verified?: </td>\n"
"<td>\n"
"<select NAME=\"joinerYN\" SIZE=1>\n"
"<option <!sel-joinerYN-N> value=\"N\">N</option>\n"
"<option <!sel-joinerYN-Y> value=\"Y\">Y</option>\n"
"</select>\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"Status:  \n"
"</td>\n"
"<td>\n"
"<TEXTAREA NAME=\"stat\", ROWS=\"5\", COLS=\"40\"><!stat></TEXTAREA>\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"Sponsor (local):\n"
"</td>\n"
"<td>\n"
"<input TYPE=\"text\" NAME=\"sponsor\" SIZE=\"40\" VALUE=\"<!sponsor>\">\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"Reviewer:\n"
"</td>\n"
"<td>\n"
"<input TYPE=\"text\" NAME=\"reviewer\" SIZE=\"40\" VALUE=\"<!reviewer>\">\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"External Source <br>or Collaborator:\n"
"</td>\n"
"<td>\n"
"<input TYPE=\"text\" NAME=\"extSource\" SIZE=\"40\" VALUE=\"<!extSource>\">\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"Open Issues:\n"
"</td>\n"
"<td>\n"
"<TEXTAREA NAME=\"openIssues\", ROWS=\"5\", COLS=\"40\"><!openIssues></TEXTAREA>\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"Notes:\n"
"</td>\n"
"<td>\n"
"<TEXTAREA NAME=\"notes\", ROWS=\"5\", COLS=\"40\"><!notes></TEXTAREA>\n"
"</td></tr>\n"
"\n"
"<tr><td align=\"right\">\n"
"&nbsp;\n"
"</td>\n"
"<td>\n"
"<input TYPE=SUBMIT NAME=\"submit\" VALUE=\"Submit\">\n"
"&nbsp;\n"
"&nbsp;\n"
"<!delbutton>\n"
"&nbsp;\n"
"&nbsp;\n"
"<!pushbutton>\n"
"&nbsp;\n"
"&nbsp;\n"
"<!clonebutton>\n"
"&nbsp;\n"
"&nbsp;\n"
"<a href=\"/cgi-bin/qaPushQ\">CANCEL</a>\n"
"</td></tr>\n"
"\n"
"</table>\n"
"\n"
"</form>\n"
"\n"
;
