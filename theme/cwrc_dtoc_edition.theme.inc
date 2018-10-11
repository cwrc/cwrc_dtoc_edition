<?php

/**
 * @file
 * Implementation of various theme and template preprocess functions.
 */

/**
 * Template preprocess for cwrc_dtoc_edition.
 *
 * @param array $variables
 *   The variables.
 */
function template_preprocess_cwrc_dtoc_edition(array &$variables) {
  $islandora_object = $variables['islandora_object'];
  $variables['islandora_content'] = '';
  if (!$islandora_object || empty($islandora_object['DTOC'])) {
    return;
  }
  if (islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['DTOC'])) {
    $datastream = $islandora_object['DTOC'];
    $data = _cwrc_dtoc_edition_xml_content_to_array($datastream->content);
    if ($data && isset($data['corpusConfiguration']['id'])) {
      $corpus_id = $data['corpusConfiguration']['id'];
      $curator_id = '';
      $options = array('query' => array('corpus' => $corpus_id));
      if (!empty($data['corpusConfiguration']['userSettings']['curation']['@attributes']['id'])) {
        $curator_id = $data['corpusConfiguration']['userSettings']['curation']['@attributes']['id'];
        $options['query']['curatorId'] = $curator_id;
      }
      $dtoc_url = url('https://voyant-tools.org/dtoc/', $options);
      $content = array(
        '#theme' => 'cwrc_visualization',
        '#url' => $dtoc_url,
      );

      // Adding the curator update js for modal callback if necessary.
      if (!empty($variables['edit_access'])) {
        $module_name = 'cwrc_dtoc_edition';
        $path = drupal_get_path('module', 'cwrc_dtoc_edition');
        $attr_id = 'cwrc-dtoc-edition-curator-form-autoload-placeholder';
        // Adding needed ctools includes and ajax stuff.
        _cwrc_dtoc_edition_reveal_form_add_ctools_includes(500, 200);
        drupal_add_library('system', 'drupal.ajax');
        // Our custom js stuff.
        $js_settings[$module_name] = array(
          'processed_curator_id' => FALSE,
          'islandora_object_id' => $islandora_object->id,
          'placeholder_attr_id' => $attr_id,
          'current_curator_id' => $curator_id,
        );
        $content['#attached']['js'][] = array('type' => 'setting', 'data' => $js_settings);
        $content['#attached']['js'][] = $path . '/js/ctools.js';
        $content['#attached']['js'][] = $path . '/js/dtoc_curator.js';
        $content['#attached']['css'][] = $path . '/css/ctools.css';
        $content['#attached']['css'][] = $path . '/css/dtoc_curator.css';
        $content['#suffix'] = '<div id="' . $attr_id . '"></div>';
      }

      $variables['islandora_content'] = drupal_render($content);
    }
  }
  $variables['theme_hook_suggestions'][] = 'islandora_basic_image__' . str_replace(':', '_', $islandora_object->id);
}
