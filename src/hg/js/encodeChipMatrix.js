/* encodeChipMatrix.js - ENCODE web app to display ChIP-seq data vs. cell types in a matrix

 Formatted: jsbeautify.py -j
 Syntax checked: jslint indent:4, plusplus: true, continue: true, unparam: true, sloppy: true, browser: true 
*/
/*global $, encodeProject */

$(function () {
    var dataType, server, requests = [
        // requests to server API
        encodeProject.serverRequests.experiment,
        encodeProject.serverRequests.cellType,
        encodeProject.serverRequests.antibody,
        encodeProject.serverRequests.expId
        ];

    var $matrixTable = $('#matrixTable');

    function handleServerData(responses) {
        // Main actions, called when loading data from server is complete

        var experiments = responses[0], 
                cellTypes = responses[1], 
                antibodies = responses[2], 
                expIds = responses[3];

        var antibodyGroups, cellTiers, expIdHash;
        var antibodyTarget, cellType;
        var matrix, antibodyTargetExps = {};

        // hide spinner and show table
        encodeMatrix.show($matrixTable);

        // set up structures for antibody targets and groups
        antibodyGroups = encodeProject.getAntibodyGroups(antibodies);

        // set up structures for cell types and their tiers
        cellTiers = encodeProject.getCellTiers(cellTypes);

        // use to filter out experiments not in this assembly
        expIdHash = encodeProject.getExpIdHash(expIds);

        // gather experiments into matrix
        matrix = makeExperimentMatrix(experiments, expIdHash, antibodyTargetExps);

       // fill in table using matrix
        encodeMatrix.tableOut($matrixTable, matrix, cellTiers, 
                        antibodyGroups, antibodyTargetExps, tableHeaderOut, rowAddCells);
    }

    function makeExperimentMatrix(experiments, expIdHash, antibodyTargetExps) {
        // Populate antibodyTarget vs. cellType array with counts of experiments

        var antibody, target, cellType;
        var matrix = {};

        $.each(experiments, function (i, exp) {
            // exclude ref genome annotations
            if (exp.cellType === 'None') {
                return true;
            }
            if (expIdHash[exp.ix] === undefined) {
                return true;
            }
            // todo: filter out with arg to hgApi ?
            if (exp.dataType !== 'ChipSeq') {
                return true;
            }
            // count experiments per target so we can prune those having none
            // (the matrix[cellType] indicates this for cell types 
            // so don't need hash for those
            antibody = encodeProject.antibodyFromExp(exp);
            if (antibody === undefined) {
                return true;
            }
            target = encodeProject.targetFromAntibody(antibody);
            if (target === undefined) {
                return true;
            }
            if (antibodyTargetExps[target] === undefined) {
                antibodyTargetExps[target] = 0;
            }
            antibodyTargetExps[target]++;

            cellType = exp.cellType;
            if (!matrix[cellType]) {
                matrix[cellType] = {};
            }
            if (!matrix[cellType][target]) {
                matrix[cellType][target] = 0;
            }
            matrix[cellType][target]++;
        });
    return matrix;
    }

    function tableHeaderOut($table, antibodyGroups, antibodyTargetExps) {
        // Generate table header and add to document

        var $tableHeader, $thead;
        var maxLen;  // for resizing header cells to accomodate label lengths
        var antibodyTarget;

        // fill in column headers from antibody targets returned by server
        $tableHeader = $('#columnHeaders');
        $thead = $('thead');

        // 1st column is row headers
        // colgroups are needed to support cross-hair hover effect
        $thead.before('<colgroup></colgroup>');

        $.each(antibodyGroups, function (i, group) {
            $tableHeader.append('<th class="groupType"><div class="verticalText">' + 
                                group.label + '</div></th>');
            maxLen = Math.max(maxLen, group.label.length);
            $thead.before('<colgroup></colgroup>');

            $.each(group.targets, function (i, target) {
                // prune out targets with no experiments 
                if (antibodyTargetExps[target] === undefined) {
                    return true;
                }
                antibodyTarget = encodeProject.getAntibodyTarget(target);
                $tableHeader.append('<th class="elementType" title="' +
                                antibodyTarget.description +
                                '"><div class="verticalText">' + target + '</div></th>');
                // add colgroup element to support cross-hair hover effect
                $thead.before('<colgroup class="experimentCol"></colgroup>');
                maxLen = Math.max(maxLen, target.length);
            });
        });
        // adjust size of headers based on longest label length
        // empirically len/2 em's is right
        $('#columnHeaders th').css('height', (String((maxLen/2 + 2)).concat('em')));
        $('#columnHeaders th').css('width', '1em');
    }

    function rowAddCells($row, antibodyGroups, antibodyTargetExps, matrix, cellType) {
        // populate a row in the matrix with cells for antibody target groups and the targets
        // null cellType indicates this is a row for a cell group (tier)

        var $td;

        $.each(antibodyGroups, function (i, group) {
            // skip group header
            $td = $('<td></td>');
            $td.addClass('matrixCell');
            $row.append($td);

            $.each(group.targets, function (i, target) {
                // prune out targets with no experiments
                if (antibodyTargetExps[target] === undefined) {
                    return true;
                }
                $td = $('<td></td>');
                $td.addClass('matrixCell');
                $row.append($td);

                if (cellType === null) {
                    return true;
                }
                if (!matrix[cellType][target]) {
                    $td.addClass('todoExperiment');
                    return true;
                }
                // this cell represents experiments that
                // fill in count, mouseover and selection by click
                $td.addClass('experiment');
                $td.text(matrix[cellType][target]);
                $td.data({
                    'target' : target,
                    'cellType' : cellType
                });
                $td.mouseover(function() {
                    $(this).attr('title', 'Click to select: ' +
                                ($(this).data().target) + ' ' + ' in ' + 
                                $(this).data().cellType +' cells');
                });
                $td.click(function() {
                    var url, antibodyTarget;

                    // TODO: encapsulate var names
                    // TODO: search on antibody
                    url = encodeMatrix.getSearchUrl(encodeProject.getAssembly());
                    url +=
                       ('&hgt_mdbVar1=dataType&hgt_mdbVal1=' + 'ChipSeq' +
                       '&hgt_mdbVar2=cell&hgt_mdbVal2=' + $(this).data().cellType +
                       '&hgt_mdbVar3=antibody');
                    // TODO: html encode ?
                    antibodyTarget = encodeProject.getAntibodyTarget($(this).data().target);
                    $.each(antibodyTarget.antibodies, function (i, antibody) {
                        url += ('&hgt_mdbVal3=' + antibody);
                    });
                    url += '&hgt_mdbVar4=view&hgt_mdbVal4=Any';
                        // TODO: open search window 
                    window.open(url, "searchWindow");
                });
            });
        });
    }

    // initialize
    encodeMatrix.start($matrixTable);

    // load data from server and set up callback
    encodeProject.loadAllFromServer(requests, handleServerData);
});
