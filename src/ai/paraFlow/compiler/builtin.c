
char *fetchBuiltInCode()
/* Return a string with the built in stuff. */
{
return
"string programName;\n"
"array of string args;\n"
"\n"
"flow punt(string message, string source=\"application\", int level=0);\n"
"flow print(var v);\n"
"to prin(var v);\n"
"to stackDump();\n"
"to keyIn() into string s;\n"
"to lineIn() into string s;\n"
"to random() into double zeroToOne;\n"
"to randInit();\n"
"flow sqrt(double x) into double y;\n"
"flow atoi(string a) into int i;\n"
"to getEnvArray() into array of string envArray;\n"
"flow milliTicks() into long milliseconds;\n"
"flow floatToString(double f, int digitsBeforeDecimal=0, \n"
"		int digitsAfterDecimal=2, bit scientificNotation=0) \n"
"into (string s);\n"
"flow intToString(long l, int minWidth, bit zeroPad, bit commas) into (string s);\n"
"\n"
"class file\n"
"    {\n"
"    string name;\n"
"    to close();\n"
"    to write(string s);\n"
"    to read(int count) into (string s);\n"
"    to readLine() into (string s);\n"
"    to readAll() into (string s);\n"
"    to put(var v);\n"
"    to get(var justForType) into var x;\n"
"    to seek(long pos, bit fromEnd=0);\n"
"    to skip(long amount);\n"
"    to tell() into long pos;\n"
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
"    flow dupe() into string dupe;\n"
"    flow start(int size) into string start;\n"
"    flow rest(int start) into string rest;\n"
"    flow middle(int start, int size) into string part;\n"
"    flow end(int size) into string end;\n"
"    flow upper();\n"
"    flow lower();\n"
"    flow append(string s);\n"
"    flow find(string s) into int foundPos;\n"
"    flow findNext(string s, int startPos) into int foundPos;\n"
"    flow words() into array of string words;\n"
"    flow tokens() into array of string spaced;\n"
"    flow nextWord(int pos) into (string s, int newPos);\n"
"    flow nextToken(int pos) into (string s, int newPos);\n"
"    flow fitLeft(int size) into (string s);\n"
"    flow fitRight(int size) into (string s);\n"
"    flow startsWith(string prefix) into bit bool;\n"
"    flow endsWith(string suffix) into bit bool;\n"
"    flow asInt() into (int x);\n"
"    flow asLong() into (long x);\n"
"    flow asDouble() into (double x);\n"
"    }\n"
"class _pf_elType\n"
"    {\n"
"    }\n"
"class _pf_array\n"
"    {\n"
"    int size;\n"
"    _operator_ append(_pf_elType el);\n"
"    to sort(var of flow (_pf_elType a, _pf_elType b) into (int v) cmp = nil);\n"
"    to push(_pf_elType el);\n"
"    }\n"
"class _pf_dir\n"
"    {\n"
"    to keys() into array of string keys;\n"
"    }\n"
;
}
