/* encodeDataMatrix.js - pull experiment table and metadata from server 
      and display in data matrix

 Formatted: jsbeautify.py -j
 Syntax checked: jslint indent:4, plusplus: true, continue: true, unparam: true, sloppy: true, browser: true
*/
/*global $, encodeProject */

$(function () {
    var requests = [
    // Requests to server API
    encodeProject.serverRequests.experiment, encodeProject.serverRequests.dataType, encodeProject.serverRequests.cellType],
        dataType, organism, assembly, server, axisLabel;

    function tableOut(matrix, cellTiers, cellTypeHash, dataGroups, dataTypeTermHash, dataTypeLabelHash) {
        // Create table where rows = cell types and columns are datatypes
        // create table and first row 2 rows (column title and column headers)
        var table, tableHeader, row, td;
        var elementHeaderLabel = "Functional Element Assays";
        var cellHeaderLabel = "Cell Types";

        table = $('<table id="matrixTable"><thead>' + '<tr id="headerLabelRow"><td></td>' + '<td id="elementHeaderLabel" class="axisType" colspan=6 title="Click to view information about all assays">' + '<a href="/cgi-bin/hgEncodeVocab?ra=encode/cv.ra&type=dataType">' + elementHeaderLabel + '</td></tr>' + '<tr id="columnHeaders">' + '<td id="cellHeaderLabel" class="axisType"' + 'title="Click to view information about all cell types">' + '<a href="/cgi-bin/hgEncodeVocab?ra=encode/cv.ra&type=Cell+Line&organism=' + organism + '">' + cellHeaderLabel + '</td></tr>' + '</thead><tbody>');
        tableHeader = $('#columnHeaders', table);
        $.each(dataGroups, function (i, group) {
            tableHeader.append('<th class="groupType"><div class="verticalText">' + group.label + '</div></th>');
            $.each(group.dataTypes, function (i, dataTypeLabel) {
                dataType = dataTypeLabelHash[dataTypeLabel].term;

                // prune out datatypes with no experiments
                if (dataTypeLabelHash[dataTypeLabel].count !== undefined) {
                    tableHeader.append('<th class="elementType" title="' + dataTypeLabelHash[dataTypeLabel].description + '"><div class="verticalText">' + dataTypeLabel + '</div></th>');
                }
            });
        });
        // add rows with cell type labels and matrix elements for indicating if
        //  there's an experiment 
        $.each(cellTiers, function (i, tier) {
            //skip bogus 4th tier (not my property ?)
            if (tier === undefined) {
                return true;
            }
            // td or th here ?
            table.append($('<tr class="matrix"><th class="groupType">' + "Tier " + tier.term + '</th></td></tr>'));
            $.each(tier.cellTypes, function (i, cellType) {
                if (!cellType) {
                    return true;
                }
                if (!matrix[cellType]) {
                    return true;
                }
                row = $('<tr><th class="elementType" title="' + cellTypeHash[cellType].description + '"><a href="/cgi-bin/hgEncodeVocab?ra=encode/cv.ra&term=' + cellType + '">' + cellType + '</a><span title="karyotype: ' + cellTypeHash[cellType].karyotype + '" class="' + cellTypeHash[cellType].karyotype + '">&bull;</span></th>');

                $.each(dataGroups, function (i, group) {
                    // skip group header
                    row.append('<td></td>');
                    $.each(group.dataTypes, function (i, dataTypeLabel) {
                        // TODO: change this class to matrixElementType
                        //$(".cellType").click(matrixClickHandler);
                        //"searchWindow");
                        dataType = dataTypeLabelHash[dataTypeLabel].term;
                        // prune out datatypes with no experiments
                        if (dataTypeLabelHash[dataTypeLabel].count === undefined) {
                            return true;
                        }
                        td = '<td class="cellType';
                        if (matrix[cellType][dataType]) {
                            td += ' experiment';
                        }
                        td += '">';
                        if (matrix[cellType][dataType]) {
                            td += '<a target="searchWindow" title="';
                            td += dataTypeLabel + ' in ' + cellType + ' cells "';
                            td += 'href="http://genome-preview.ucsc.edu/cgi-bin/hgTracks?db=hg19&tsCurTab=advancedTab&hgt_tsPage=&hgt_tSearch=search&hgt_mdbVar1=cell&hgt_mdbVar2=dataType&hgt_mdbVar3=view&hgt_mdbVal3=Any&hgt_mdbVal1=';
                            td += cellType;
                            td += '&hgt_mdbVal2=';
                            td += dataType;
                            //td += '"><font color=#00994D>';
                            td += '"><font>';
                            td += matrix[cellType][dataType];
                            //td += ".....";
                            td += '</font></a>';
                            td += '</td>';
                        }
                        row.append(td);
                    });
                    table.append(row);
                });
                table.append(row);
            });
        });
        table.append('</tbody>');
        $("body").append(table);

        // use floating-table-header plugin
        table.floatHeader({
            cbFadeIn: function (header) {
                // hide axis labels -- a bit tricy to do so
                // as special handling needed for X axis label
                $(".floatHeader #headerLabelRow").remove();
                $(".floatHeader #cellHeaderLabel").html('');

                // Note: user-defined callback requires 
                // default actions from floatHeader plugin
                // implementation (stop+fadeIn)
                header.stop(true, true);
                header.fadeIn(100);

                // save label to restore when scrolled back to top
                //axisLabel = $(".axisType").html();
                //$(".axisType").html('');
            }
/*,
            cbFadeOut: function (header) {
                // show elements with class axisType
                header.stop(true, true)
                $(".axisType").html(axisLabel);
                header.fadeOut(100);
            }
            */
        });
    }

    function handleServerData(responses) {
        // Main actions, called when loading data from server is complete
        var experiments = responses[0],
            dataTypes = responses[1],
            cellTypes = responses[2],
            dataTypeLabelHash = {},
            dataTypeTermHash = {},
            cellTypeHash = {},
            dataType, cellType, matrix = {},
            dataGroups = {},
            cellTiers, header;

        // page-specific variables from calling page
        organism = encodeDataMatrix_organism;
        assembly = encodeDataMatrix_assembly;
        header = encodeDataMatrix_pageHeader;

        $("#pageHeader").text(header);
        $("title").text('ENCODE ' + header);

        // set up structures for data types and their groups
        $.each(dataTypes, function (i, item) {
            dataTypeTermHash[item.term] = item;
            dataTypeLabelHash[item.label] = item;
        });
        // data type labels tucked into their tiers
        dataGroups = encodeProject.getDataGroups(dataTypes);

        // set up structures for cell types and their tiers
        $.each(cellTypes, function (i, item) {
            cellTypeHash[item.term] = item;
        });
        cellTiers = encodeProject.getCellTiers(cellTypes);

        // gather experiments into matrix
        $.each(experiments, function (i, exp) {
            // todo: filter out with arg to hgApi
            if (exp.organism !== organism) {
                return true;
            }
            // exclude ref genome annotations
            if (exp.cellType === 'None') {
                return true;
            }
            // count experiments per dataType so we can prune those having none
            // (the matrix[cellType] indicates this for cell types 
            // so don't need hash for those
            dataType = exp.dataType;
            if (!dataTypeTermHash[dataType].count) {
                dataTypeTermHash[dataType].count = 0;
            }
            dataTypeTermHash[dataType].count++;

            cellType = exp.cellType;
            if (!matrix[cellType]) {
                matrix[cellType] = {};
            }
            if (!matrix[cellType][dataType]) {
                matrix[cellType][dataType] = 0;
            }
            matrix[cellType][dataType]++;
        });

        // fill in table
        tableOut(matrix, cellTiers, cellTypeHash, dataGroups, dataTypeTermHash, dataTypeLabelHash);
    }

    // initialize encodeProject
    // get server from calling web page (intended for genome-preview)
    if ('encodeDataMatrix_server' in window) {
        server = encodeDataMatrix_server;
    } else {
        server = document.location.hostname;
        // or document.domain ?
    }

    encodeProject.setup({
        server: server
    });
    encodeProject.loadAllFromServer(requests, handleServerData);
});
