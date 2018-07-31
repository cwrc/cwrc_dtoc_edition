/**
 * Provide the HTML to create the modal dialog.
 */
Drupal.theme.prototype.CToolsCWRCDtocEditionModal = function () {
  var html = '';

  html += '<div id="ctools-modal" class="popups-box">';
  html += '  <div class="ctools-modal-content ctools-cwrc-dtoc-edition-modal-content">';
  html += '    <table cellpadding="0" cellspacing="0" id="ctools-face-table">';
  html += '      <tr>';
  html += '        <td class="popups-tl popups-border"></td>';
  html += '        <td class="popups-t popups-border"></td>';
  html += '        <td class="popups-tr popups-border"></td>';
  html += '      </tr>';
  html += '      <tr>';
  html += '        <td class="popups-cl popups-border"></td>';
  html += '        <td class="popups-c" valign="top">';
  html += '          <div class="popups-container">';
  html += '            <div class="modal-header popups-title">';
  html += '              <span id="modal-title" class="modal-title"></span>';
  html += '              <span class="popups-close"><a class="close" href="#">' + Drupal.CTools.Modal.currentSettings.closeText + '</a></span>';
  html += '              <div class="clear-block"></div>';
  html += '            </div>';
  html += '            <div class="modal-scroll"><div id="modal-content" class="modal-content popups-body"></div></div>';
  html += '            <div class="popups-buttons"></div>'; //Maybe someday add the option for some specific buttons.
  html += '            <div class="popups-footer"></div>'; //Maybe someday add some footer.
  html += '          </div>';
  html += '        </td>';
  html += '        <td class="popups-cr popups-border"></td>';
  html += '      </tr>';
  html += '      <tr>';
  html += '        <td class="popups-bl popups-border"></td>';
  html += '        <td class="popups-b popups-border"></td>';
  html += '        <td class="popups-br popups-border"></td>';
  html += '      </tr>';
  html += '    </table>';
  html += '  </div>';
  html += '</div>';

  return html;
};

(function ($, Drupal) {
  'use strict';

  Drupal.behaviors.cwrcDtocEditionAjaxCtools = {
    attach: function (context, settings) {
      var form_id = 'cwrc_dtoc_edition_ctools_modal_collection_files_selector_form';
      // Listen to the click event on the parent and then trigger mousedown.
      // @todo check if touchstart needs to be supported.
      $('.cwrc-dtoc-edition--reveal-form--updated-collection-action-trigger', context).on('click', function() {
        $(this).trigger('mousedown');
      });

      $(document).ajaxComplete(function(e, r, s) {
        var params = {},
          collection_pid = '';
        if (typeof s.data !== 'undefined' && s.data.includes(form_id)) {
          console.log('Starting the search...');
          params = get_url_params(s.data);
          collection_pid = params.collection_pid;
        }
        if (collection_pid) {
          // Updating the parent which was selected.
          $('input[name="sources[collections][updated_collection_pid]"]').val(collection_pid);
          $('.cwrc-dtoc-edition--reveal-form--updated-collection-action-trigger', context).trigger('click');
        }
      });

      /**
       * Helper function to get the parent id from the ajax post return.
       *
       * @param query
       * @returns {{collection_id: string}}
       */
      function get_url_params(query) {
        var match,
          urlParams = {
            collection_pid: ''
          },
          pl = /\+/g,
          search = /([^&=]+)=?([^&]*)/g,
          decode = function (s) { return decodeURIComponent(s.replace(pl, " ")); };

        while (match = search.exec(query)) {
          urlParams[decode(match[1])] = decode(match[2]);
          // Once the form id and the parent id keys have been assigned we done with this loop.
          if (typeof urlParams.form_id !== 'undefined' && urlParams.collection_pid) {
            break;
          }
        }

        return urlParams;
      }
    }
  };
})(jQuery, Drupal);
