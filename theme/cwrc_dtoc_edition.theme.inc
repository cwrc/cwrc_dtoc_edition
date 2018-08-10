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
    $data = drupal_json_decode(drupal_json_encode((array) simplexml_load_string($datastream->content)));
    if ($data && isset($data['corpusConfiguration']['id'])) {
      $corpus_id = $data['corpusConfiguration']['id'];
      $options = array('query' => array('corpus' => $corpus_id));
      $dtoc_url = url('https://voyant-tools.org/dtoc/', $options);
      $content = array(
        '#theme' => 'cwrc_visualization',
        '#url' => $dtoc_url,
      );
      $variables['islandora_content'] = drupal_render($content);
    }
  }
  $variables['theme_hook_suggestions'][] = 'islandora_basic_image__' . str_replace(':', '_', $islandora_object->id);
}
