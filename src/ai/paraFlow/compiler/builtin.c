
char *fetchBuiltinCode()
/* Return a string with the built in stuff. */
{
return
"string programName;\n"
"array of string args;\n"
"\n"
"to die(string message);\n"
"to print(var v);\n"
"to prin(var v);\n"
"to floatString(double f, int digitsBeforeDecimal, int digitsAfterDecimal,\n"
"               bit scientificNotation) into (string s);\n"
"to intString(long l, int minWidth, bit zeroPad, bit commas) into (string s);\n"
"\n"
"class file\n"
"    {\n"
"    string name;\n"
"    to writeString(string s);\n"
"    to readLine() into (string s);\n"
"    to readBytes(int count) into (string s);\n"
"    }\n"
"\n"
"to fileOpen(string name, string mode) into file f;\n"
;
}

char *fetchStringDef()
/* Return a string with definition of string. */
{
return
"class _pf_string\n"
"    {\n"
"    int size;\n"
"    to dupe() into string dupe;\n"
"    to start(int size) into string start;\n"
"    to rest(int start) into string rest;\n"
"    to middle(int start, int size) into string part;\n"
"    to end(int size) into string end;\n"
"    to upper();\n"
"    to lower();\n"
"    to append(string s);\n"
"    to find(string s) into int found;\n"
"    to findNext(string s, int startPos) into int found;\n"
"    to nextSpaced(int pos) into (int newPos, string s);\n"
"    to nextWord(int pos) into (int newPos, string s);\n"
"    to fitLeft(int size) into (string s);\n"
"    to fitRight(int size) into (string s);\n"
"    }\n"
"class _pf_array\n"
"    {\n"
"    int size;\n"
"    }\n"
;
}
