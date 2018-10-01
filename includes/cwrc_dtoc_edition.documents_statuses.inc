<?php

/**
 * @file
 * Holds the dtoc reveal form.
 */

function cwrc_dtoc_edition_refresh_get_documents_statuses(AbstractFedoraDatastream $dtoc_datastream = NULL) {
  $statuses = array();
  $has_update = FALSE;
  if ($dtoc_datastream) {
    $data = drupal_json_decode(drupal_json_encode((array) simplexml_load_string($dtoc_datastream->content)));
    $collections = empty($data['sources']['collection']['@attributes']) && count($data['sources']['collection']) > 1 ? $data['sources']['collection'] : $data['sources'];

    foreach ($collections as $collection) {
      $documents = !empty($collection['document']) ? $collection['document'] : array();
      $collection_pid = $collection['@attributes']['pid'];
      $collection_label = $collection['@attributes']['label'];
      $statuses[$collection_pid] = array(
        'label' => $collection_label,
        'documents' => array(),
      );
      foreach ($documents as $document) {
        $pid = $document['@attributes']['pid'];
        $created_time = !empty($document['@attributes']['createdTimestamp']) ? $document['@attributes']['createdTimestamp'] : 0;
        // @todo find out how to deal with deleted documents.
        if ($created_time && $current_doc = islandora_object_load($pid)) {
          /** @var \IslandoraFedoraDatastream $current_doc_datastream */
          $current_doc_datastream = $current_doc['CWRC'];
          if ($current_doc_datastream->createdDate->getTimestamp() > $created_time) {
            $has_update = TRUE;
            $statuses[$collection_pid]['documents'][$pid] = $current_doc->label;
          }
        }
      }
    }
  }

  return $has_update ? $statuses : array();
}

function cwrc_dtoc_edition_refresh_get_collections(array $data) {
  $saved_collections = empty($data['sources']['collection']['@attributes']) && count($data['sources']['collection']) > 1 ? $data['sources']['collection'] : $data['sources'];
  $collections = array();
  foreach ($saved_collections as $collection) {
    $collection_pid = $collection['@attributes']['pid'];
    $collections[$collection_pid] = array(
      'pid' => $collection_pid,
      'label' => $collection['@attributes']['label'],
      'selected_files' => array(),
    );
    $documents = !empty($collection['document']) ? $collection['document'] : array();
    foreach ($documents as $document) {
      $document_pid = $document['@attributes']['pid'];
      $collections[$collection_pid]['selected_files'][$document_pid] = $document['@attributes']['label'];
    }
  }
  return $collections;
}

function cwrc_dtoc_edition_refresh_get_corpus_configurations(array $data) {
  $corpus_configurations = array();
  $user_settings = $data['corpusConfiguration']['userSettings'];
  $is_custom_xpaths = $user_settings['@attributes']['type'] === 'custom';
  if ($is_custom_xpaths) {
    $corpus_configurations['custom']['optin'] = $is_custom_xpaths;
    // Adding custom xpaths.
    $corpus_configurations['custom']['options']['xpaths'] = array();
    if (!empty($data['corpusConfiguration']['xmlXpathExpressions'])) {
      foreach ($data['corpusConfiguration']['xmlXpathExpressions'] as $xpath_key => $xpath_expression) {
        $xpath_key_form = strtolower(substr($xpath_key, 0, -5));
        $corpus_configurations['custom']['options']['xpaths'][$xpath_key_form] = $xpath_expression;
      }
    }
    // Curation information. @todo find a way to incorporate custom part.
    $corpus_configurations['custom']['options']['curation'] = $user_settings['curation']['@attributes'];
  }
  else {
    $is_from_cwrc_dtoc_edition = $user_settings['@attributes']['type'] === 'copy_cwrc_edition';
    $corpus_configurations['copy_cwrc_edition']['optin'] = $is_from_cwrc_dtoc_edition;
    $is_from_voyant_dtoc_edition = $user_settings['@attributes']['type'] === 'copy_dtoc_edition';
    $corpus_configurations['copy_dtoc_edition']['optin'] = $is_from_voyant_dtoc_edition;
    $value = !empty($user_settings['@attributes']['value']) ? $user_settings['@attributes']['value'] : '';
    $corpus_configurations['copy_cwrc_edition']['search'] = $value;
    $corpus_configurations['copy_dtoc_edition']['search'] = $value;
  }
  return $corpus_configurations;
}
