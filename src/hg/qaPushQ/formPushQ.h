char formQ[] =
""
"<h1> Push Queue Form"
""
"<form ACTION=\"/cgi-bin/qaPushQ\" NAME=\"mainForm\" METHOD=\"POST\">"
""
""
"<input TYPE=\"hidden\" NAME=\"action\" VALUE=\"post\"  >"
"<input TYPE=\"hidden\" NAME=\"qid\"    VALUE=\"<!qid>\">"
""
"<table cellpadding=6>"
""
"<tr><td align=\"right\">"
"Priority:"
"</td>"
"<td>"
"<select NAME=\"priority\" SIZE=1>"
"<option <!sel-priority-A> value=\"A\">A</option>"
"<option <!sel-priority-B> value=\"B\">B</option>"
"<option <!sel-priority-C> value=\"C\">C</option>"
"<option <!sel-priority-L> value=\"L\">L</option>"
"</select>"
"</td></tr>"
""
"<tr><td align=\"right\">"
"Date Opened:"
"</td>"
"<td>"
"<input TYPE=\"text\" NAME=\"qadate\" SIZE=\"10\" VALUE=\"<!qadate>\">"
"</td></tr>"
""
"<tr><td align=\"right\">"
"New track?:"
"</td>"
"<td>"
"<select NAME=\"newYN\" SIZE=1>"
"<option <!sel-newYN-N> value=\"N\">N</option>"
"<option <!sel-newYN-Y> value=\"Y\">Y</option>"
"</select>"
"</td></tr>"
""
"<tr><td align=\"right\">"
"Track:"
"</td>"
"<td>"
"<TEXTAREA NAME=\"track\", ROWS=\"4\", COLS=\"40\"><!track></TEXTAREA>"
"</td></tr>"
""
"<tr><td align=\"right\">"
"Databases:"
"</td>"
"<td>"
"<input TYPE=\"text\" NAME=\"dbs\" SIZE=\"40\" VALUE=\"<!dbs>\">"
"</td></tr>"
""
"<tr><td align=\"right\">"
"Tables:"
"</td>"
"<td>"
"<TEXTAREA NAME=\"tbls\", ROWS=\"5\", COLS=\"40\"><!tbls></TEXTAREA>"
"</td></tr>"
""
"<tr><td align=\"right\">"
"CGIs:"
"</td>"
"<td>"
"<TEXTAREA NAME=\"cgis\", ROWS=\"5\", COLS=\"40\"><!cgis></TEXTAREA>"
"</td></tr>"
""
"<tr><td align=\"right\">"
"Files:"
"</td>"
"<td>"
"<TEXTAREA NAME=\"files\", ROWS=\"5\", COLS=\"40\"><!files></TEXTAREA>"
"</td></tr>"
""
"<tr><td align=\"right\">"
"Size(MB):"
"</td>"
"<td>"
"<input TYPE=\"text\" NAME=\"sizeMB\" SIZE=\"40\" VALUE=\"<!sizeMB>\">"
"</td></tr>"
""
"<tr><td align=\"right\">"
"Current Location:"
"</td><td>"
"<select NAME=\"currLoc\" SIZE=1>"
"<option <!sel-currLoc-hgwdev> value=\"hgwdev\">hgwdev</option>"
"<option <!sel-currLoc-hgwbeta> value=\"hgwbeta\">hgwbeta</option>"
"</select>"
""
"</td></tr>"
""
"<tr><td align=\"right\">"
"Makedoc verified?: </td>"
"<td>"
"<select NAME=\"makeDocYN\" SIZE=1>"
"<option <!sel-makeDocYN-N> value=\"N\">N</option>"
"<option <!sel-makeDocYN-Y> value=\"Y\">Y</option>"
"</select>"
"</td></tr>"
""
"<tr><td align=\"right\">"
"Online Help:"
"</td>"
"<td>"
"<input TYPE=\"text\" NAME=\"onlineHelp\" SIZE=\"40\" VALUE=\"<!onlineHelp>\">"
"</select>"
""
"</td></tr>"
""
"<tr><td align=\"right\">"
"Index verified?: </td>"
"<td>"
"<select NAME=\"ndxYN\" SIZE=1>"
"<option <!sel-ndxYN-N> value=\"N\">N</option>"
"<option <!sel-ndxYN-Y> value=\"Y\">Y</option>"
"</select>"
"</td></tr>"
""
"<tr><td align=\"right\">"
"All.Joiner verified?: </td>"
"<td>"
"<select NAME=\"joinerYN\" SIZE=1>"
"<option <!sel-joinerYN-N> value=\"N\">N</option>"
"<option <!sel-joinerYN-Y> value=\"Y\">Y</option>"
"</select>"
"</td></tr>"
""
"<tr><td align=\"right\">"
"Status:  "
"</td>"
"<td>"
"<TEXTAREA NAME=\"stat\", ROWS=\"5\", COLS=\"40\"><!stat></TEXTAREA>"
"</td></tr>"
""
"<tr><td align=\"right\">"
"Sponsor (local):"
"</td>"
"<td>"
"<input TYPE=\"text\" NAME=\"sponsor\" SIZE=\"40\" VALUE=\"<!sponsor>\">"
"</td></tr>"
""
"<tr><td align=\"right\">"
"Reviewer:"
"</td>"
"<td>"
"<input TYPE=\"text\" NAME=\"reviewer\" SIZE=\"40\" VALUE=\"<!reviewer>\">"
"</td></tr>"
""
"<tr><td align=\"right\">"
"External Source <br>or Collaborator:"
"</td>"
"<td>"
"<input TYPE=\"text\" NAME=\"extSource\" SIZE=\"40\" VALUE=\"<!extSource>\">"
"</td></tr>"
""
"<tr><td align=\"right\">"
"Open Issues:"
"</td>"
"<td>"
"<TEXTAREA NAME=\"openIssues\", ROWS=\"5\", COLS=\"40\"><!openIssues></TEXTAREA>"
"</td></tr>"
""
"<tr><td align=\"right\">"
"Notes:"
"</td>"
"<td>"
"<TEXTAREA NAME=\"notes\", ROWS=\"5\", COLS=\"40\"><!notes></TEXTAREA>"
"</td></tr>"
""
"<tr><td align=\"right\">"
"&nbsp;"
"</td>"
"<td>"
"<input TYPE=SUBMIT NAME=\"submit\" VALUE=\"Submit\">"
"&nbsp;"
"&nbsp;"
"<!delbutton>"
"&nbsp;"
"&nbsp;"
"<!pushbutton>"
"&nbsp;"
"&nbsp;"
"<!clonebutton>"
"</td></tr>"
""
"</table>"
""
"</form>"
""
;
