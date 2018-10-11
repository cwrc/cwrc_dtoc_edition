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
