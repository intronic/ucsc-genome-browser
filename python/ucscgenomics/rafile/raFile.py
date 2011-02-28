import sys
import re
import orderedDict
import raEntry

class RaFile(orderedDict.OrderedDict):
    """
    Stores an Ra file in a set of entries, one for each stanza in the file.
    """

    def read(self, filePath):
        """
        Reads an rafile, separating it by keyField, and internalizes it.

        keyField must be the first field in each entry.
        """

        file = open(filePath, 'r')
        entry = None 
        raKey = None

        for line in file:

            line = line.strip()

            # put all commented lines in the list only
            if (line.startswith('#')):
                self._ordering.append(line)
                continue

            # a blank line indicates we need to move to the next entry
            if (len(line) == 0):
                raKey = None
                entry = None
                continue

            # check if we're at the first key in a new entry
            #if (line.split()[0].strip() == keyField):
            if (entry == None): 
                if len(line.split()) < 2:
                    raise KeyError()

                raKey = line.split(' ', 1)[0].strip()
                raVal = line.split(' ', 1)[1].strip()
                entry = raEntry.RaEntry()
                entry.add(raKey, raVal)
                self.add(raVal, entry)

            # otherwise we should be somewhere in the middle of an entry
            elif (entry != None):
                raKey = line.split()[0].strip()
                raVal = ''

                if len(line.split()) > 1:
                    raVal = line.split(' ', 1)[1].strip()

                entry.add(raKey, raVal)

            # we only get here if we didn't find the keyField at the beginning
            else:
                raise KeyError()

        file.close()

    def write(self):
        """
        Write out the entire RaFile.
        """

        print self

    def writeEntry(self, key):
        """
        Write out a single stanza, specified by key
        """

        print self.getValue(key)

    def iterValues(self):
        """
        Return an iterator over the values in the dictionary
        """

        for item in self._ordering:
            if item.startswith('#'):
                continue
            yield self.getValue(item)

    def __str__(self):
        str = ''
        for key in self._ordering:
            if key.startswith('#'):
                str = str + key + '\n'
            else:
                str = str + self._dictionary[key].__str__() + '\n'
        return str
