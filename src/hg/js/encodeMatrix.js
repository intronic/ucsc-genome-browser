/* encodeMatrix.js - shared code for ENCODE data matrix apps

 Formatted: jsbeautify.py -j
 Syntax checked: jslint indent:4, plusplus: true, continue: true, unparam: true, sloppy: true, browser: true
*/
/*global $, encodeProject */

var encodeMatrix = (function () {

    // spinning image displayed during AJAX activity
    var spinner;

    function addSearchPanel($div, isFile) {
        // Create panel of radio buttons for user to change search type
        // isFileSearch determines initial setting
        // Add to passed in div ID; e.g. #searchTypePanel
        $div.append('<span id="searchPanelInstructions">search for:&nbsp;</span><input type="radio" name="searchType" id="searchTracks" value="tracks" onclick="encodeMatrix.setFileSearch(false);">tracks<input type="radio" name="searchType" id="searchFiles" value="files" onclick="encodeMatrix.setFileSearch(true);" >files');
        if (isFile) {
            $('#searchFiles').attr('checked', true);
        } else {
            $('#searchTracks').attr('checked', true);
        }
        encodeMatrix.setFileSearch(isFile);
    }

    return {

        // UI panel for search: select tracks or files

        setFileSearch: function (isFile) {
            // Set search type cookie to retain user choice
            document.cookie = "encodeMatrix.search=" + (isFile ? "file" : "track");
        },

        isFileSearch: function () {
            // Check search type cookie to retain user choice
            // Defaults to track search if no cookie set
            return document.cookie.match(/encodeMatrix.search=file/);
        },

        getSearchUrl: function () {
            // Format a URL for search of type requested in search panel
            // Args are objects having mdbVar/mdbVal properties
            // Set each arg mdbValN to mdbVarN, empty out remaining vals to a total of 6
            // (cheaper than querying for actual number)

            var prog, cartVar, url, i, j;
            var argsLen = arguments.length;
            var mdbVals = [];

            if ($('input:radio[name=searchType]:checked').val() === "tracks") {
                prog = 'hgTracks';
                cartVar = 'hgt_tSearch';
            } else {
                prog = "hgFileSearch";
                cartVar = "fsFileType=Any&hgfs_Search";
            }
             url = '/cgi-bin/' + prog + '?db=' + encodeProject.getAssembly() + 
                '&' + cartVar + '=search' + '&tsCurTab=advancedTab&hgt_tsPage=' +
                '&tsName=&tsDescr=&tsGroup=Any';
            for (i = 0; i < argsLen; i += 1) {
                url += '&hgt_mdbVar' + (i + 1) + '=' + arguments[i].mdbVar;
                // can pass an array or a single string -- so force to array for uniform handling
                // Search sees multiple mdbValN= variables for the same mdbVarN as a list of vals
                mdbVals = [].concat(arguments[i].mdbVal);
                for (j = 0; j < mdbVals.length; j++) {
                    url += '&hgt_mdbVal' + (i + 1) + '=' + mdbVals[j];
                }
            }
            url += '&hgt_mdbVar' + (argsLen + 1) + '=view' +
                    '&hgt_mdbVal' + (argsLen + 1) + '=Any';
            for (i = argsLen + 1; i < 6; i += 1) {
                url += '&hgt_mdbVar' + (i + 1) + '=[]';
            }
            return (url);
        },

        // General purpose functions

        start: function ($el) {
            // Initialize application. Caller passes in jQuery selector object to hide/show

            // get server from calling web page (e.g. can sub in genome-preview)
            if ('encodeMatrix_server' in window) {
                server = encodeMatrix_server;
            } else {
                server = document.location.hostname;
                // or document.domain ?
            }
            // variables from calling page
            organism = encodeMatrix_organism;
            assembly = encodeMatrix_assembly;
            $('#assemblyLabel').text(assembly);
            header = encodeMatrix_pageHeader;
            $('#pageHeader').text(header);
            document.title = 'ENCODE ' + header;

            encodeProject.setup({
                server: server,
                assembly: assembly
            });

            // show only spinner until data is retrieved
            $el.hide();
            $('.helpLauncher').hide();
            $('.xIcon').hide();

            // setup click handlers for help controls
            $('.xIcon').click(function() {
                $('.helpText').toggle();
                $('.helpLauncher').toggle();
            });
            $('.helpLauncher').click(function() {
                $('.helpText').toggle();
                $('.helpLauncher').toggle();
            });
            spinner = showLoadingImage('spinner', true);

            // add radio buttons for search type to specified div on page
            addSearchPanel($('#searchTypePanel'), encodeMatrix.isFileSearch());

        },

    show: function ($el) {
        // Call after data loads to show display
        hideLoadingImage(spinner);
        $('#spinner').remove();
        $('.xIcon').show();
        $el.show();
        },

    // Table rendering special effects

    addTableFloatingHeader: function ($table) {
        // add callback for floating table header feature

       // NOTE: it may be possible to revive floating header functionality in IE 
       // using this plug-in, but I've timed out 
       // (not able to make it work in simple HTML either).

          $table.floatHeader({
            cbFadeIn: function (header) {
                // hide axis labels -- a bit tricky to do so
                // as special handling needed for X axis label
                $('.floatHeader #headerLabelRow').remove();
                $('.floatHeader #cellHeaderLabel').html('');
                $('.floatHeader #searchTypePanel').remove();

                // Note: user-defined callback requires 
                // default actions from floatHeader plugin
                // implementation (stop+fadeIn)
                header.stop(true, true);
                header.fadeIn(100);

                if (encodeMatrix.isFileSearch()) {
                    $('#searchFiles').attr('checked', true);
                } else {
                    $('#searchTracks').attr('checked', true);
                }
            }
        });
    },

    hoverExperiment: function ($td) {
        // Add highlight on hover of experiment table cell.
        // NOTE: suppressed in IE due to performance issues
        // NOTE: tied to class name in dataMatrix.css

        if (!$.browser.msie) {
            $td.hover(
                function() {
                    $(this).addClass("experimentHover");
                },
                function () {
                    $(this).removeClass("experimentHover");
                }
            );
        }
    },

    hoverTableCrossHair: function ($table) {
        // Add Cross-hair effect (column and row hover) to table
        // thanks to Chris Coyier, css-tricks.com
        // NOTE:  acts on colgroups declared at start of table
        // NOTE: uses many hard-coded CSS classes, and adds 'crossHair' class (relies on styling)

        var $col;
        var tableId, tableSelector;

        // NOTE: too slow on IE, so skip 
        if ($.browser.msie) {
            return;
        }
        tableId = $table.attr('id');
        // add event handling to both main table and its clone
        // NOTE: second table name is generated from floatheader plugin
        tables = '#' + tableId + ',' + '#' + tableId + 'FloatHeaderClone';

        $(tables).delegate('.matrixCell, .elementType','mouseover mouseleave', function(ev) {
            if (!$(this).hasClass('experiment') && !$(this).hasClass('todoExperiment') &&
                !$(this).hasClass('elementType') && !$(this).hasClass('groupType')) {
                return;
            }
            if (ev.type == 'mouseover') {
                // refrain from highlighting header row
                if (!$(this).parent().is('#columnHeaders')) {
                    $(this).parent().addClass('crossHair');
                }
                $col = $('colGroup').eq($(this).index());
                if ($col.hasClass('experimentCol')) {
                    $col.addClass('crossHair');
                }
            } else {
                $(this).parent().removeClass('crossHair');
                $('colGroup').eq($(this).index()).removeClass('crossHair');
            }
        });
    },

    rotateTableCells: function ($table) {
        // plugin from David Votrubec, handles IE rotate
        if ($.browser.msie) {
            $table.rotateTableCellContent({className: 'verticalText'});
            $(this).attr('disabled', 'disabled');
        }
    },

    tableMatrixOut: function ($table, matrix, cellTiers, groups, expCounts, rowAddCells) {
        // Fill in matrix --
        // add rows with cell type labels (column 1) and cells for experiments
        // add sections for each Tier of cell type

        var maxLen, karyotype, cellType;
        var $row;

        // add sections for each Tier of cells
        $.each(cellTiers, function (i, tier) {
            //skip bogus 4th tier (not my property ?)
            if (tier === undefined) {
                return true;
            }
            $row = $('<tr class="matrix"><th class="groupType">' +
                                "Tier " + tier.term + '</th></td></tr>');
            rowAddCells($row, groups, expCounts, matrix, null);
            $table.append($row);
            maxLen = 0;

            $.each(tier.cellTypes, function (i, term) {
                if (!term) {
                    return true;
                }
                if (!matrix[term]) {
                    return true;
                }
                cellType = encodeProject.getCellType(term);
                karyotype = cellType.karyotype;
                if (karyotype !== 'cancer' && karyotype !== 'normal') {
                    karyotype = 'unknown';
                }
                // note karyotype bullet layout requires non-intuitive placement
                // in code before the span that shows to it's left
                $row = $('<tr>' +
                    '<th class="elementType">' +
                    '<span style="float:right; text-align: right;" title="karyotype: ' + 
                    karyotype + '" class="karyotype ' + karyotype + '">&bull;</span>' +
                    '<span title="' + cellType.description + 
                    '"><a target="cvWindow" href="/cgi-bin/hgEncodeVocab?ra=encode/cv.ra&deprecated=true&term=' + 
                    encodeURIComponent(cellType.term) + '">' + cellType.term + '</a>' +
                    '</th>'
                    );
                maxLen = Math.max(maxLen, cellType.term.length);

                rowAddCells($row, groups, expCounts, matrix, cellType.term);
                $table.append($row);
            });
            // adjust size of row headers based on longest label length
            $('tbody th').css('height', '1em');
            $('tbody th').css('width', (String((maxLen/2 + 2)).concat('em')));
        });
        $('body').append($table);
    },

    tableOut: function ($table, matrix, cellTiers, groups, expCounts, tableHeaderOut, rowAddCells) {
        // Create table with rows for each cell type and columns for each antibody target

        tableHeaderOut($table, groups, expCounts);
        encodeMatrix.tableMatrixOut($table, matrix, cellTiers, groups, expCounts, rowAddCells);
        encodeMatrix.addTableFloatingHeader($table);
        encodeMatrix.rotateTableCells($table);
        encodeMatrix.hoverTableCrossHair($table);
    }
};
}());
