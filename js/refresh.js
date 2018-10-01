(function ($, Drupal) {

  'use strict';

  Drupal.behaviors.cwrcDtocEditionRefresh = {
    attach: function (context, settings) {
      var module_settings = settings.cwrc_dtoc_edition;
      var $lazy_loader = $('.' + module_settings.lazy_load_trigger_class, context);
      if (module_settings.trigger_lazy_load && !$lazy_loader.hasClass('lazy-load-trigger-processed')) {
        $lazy_loader.hide()
          .prop('checked', true)
          .addClass('lazy-load-trigger-processed')
          .trigger("change");
      }
    }
  };
})(jQuery, Drupal);
