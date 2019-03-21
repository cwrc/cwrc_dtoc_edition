<?php

/**
 * @file
 * Holds the dtoc reveal form.
 */

function cwrc_dtoc_edition_refresh_get_documents_statuses(AbstractFedoraDatastream $dtoc_datastream = NULL) {
  $statuses = array();
  $has_update = FALSE;

  if ($dtoc_datastream) {
    $data = _cwrc_dtoc_edition_xml_content_to_array($dtoc_datastream->content);
    $collections = empty($data['sources']['collection']['@attributes']) && count($data['sources']['collection']) > 1 ? $data['sources']['collection'] : $data['sources'];

    foreach ($collections as $collection) {
      $documents = !empty($collection['document']) ? $collection['document'] : array();
      $collection_pid = $collection['@attributes']['pid'];
      $collection_label = $collection['@attributes']['label'];
      $statuses[$collection_pid] = array(
        'label' => $collection_label,
        'documents' => array(),
      );

      if (isset($documents['@attributes']['pid'])) {
        $pid = $documents['@attributes']['pid'];
        $created_time = !empty($documents['@attributes']['createdTimestamp']) ? $documents['@attributes']['createdTimestamp'] : 0;
        if ($created_time && $current_doc = islandora_object_load($pid)) {
          /** @var \IslandoraFedoraDatastream $current_doc_datastream */
          $current_doc_datastream = $current_doc['CWRC'];
          if ($current_doc_datastream->createdDate->getTimestamp() > $created_time) {
            $has_update = TRUE;
            $statuses[$collection_pid]['documents'][$pid] = $current_doc->label;
          }
        }
      }
      else {
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
  }

  return $has_update ? $statuses : array();
}
