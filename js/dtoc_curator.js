(function ($, Drupal) {

  'use strict';

  Drupal.behaviors.cwrcDtocEditionCuratorUpdate = {
    attach: function (context, settings) {
      var module_settings = settings.cwrc_dtoc_edition;
      var element = $('#' + module_settings.placeholder_attr_id, context);
      var current_curator_id = module_settings.current_curator_id;

      function dtocIframeWindowHistoryUpdated(msg) {
        if (msg.origin !== 'https://voyant-tools.org' || module_settings.processed_curator_id) {
          return;
        }

        var messageData = typeof msg.data !== 'object' ? JSON.parse(msg.data) : msg.data;
        if (messageData.hasOwnProperty('curatorId') && current_curator_id !== messageData.curatorId) {
          // Making sure that we are not re running this by marking thar we have
          // processed the curator id.
          module_settings.processed_curator_id = true;
          module_settings.initially_load = false;

          var curatorId = messageData.curatorId;
          var base = '/dtoc-edition/curator-id-update/' + module_settings.islandora_object_id + '/' + curatorId + '/nojs';
          var element_settings = {};
          element_settings.url = base;
          element_settings.event = 'none';
          element_settings.progress = { type: 'throbber'};

          // Instantiating the drupal ajax object.
          Drupal.ajax[base] = new Drupal.ajax(base, element, element_settings);
          Drupal.ajax[base].beforeSerialize(Drupal.ajax[base].element, Drupal.ajax[base].options);

          // Making sure that ctools autoload use our style.
          Drupal.CTools.Modal.show('ctools-cwrc-dtoc-edition-style');
          // Running the ajax.
          $.ajax(Drupal.ajax[base].options);
        }
      }

      window.addEventListener('message', dtocIframeWindowHistoryUpdated, false);
    }
  };
})(jQuery, Drupal);
