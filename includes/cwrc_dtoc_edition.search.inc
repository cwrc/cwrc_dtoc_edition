<?php

/**
 * @file
 * Holds dtoc edition search helper functionalities.
 */

/**
 * Menu callback for the DToC Edition search autocomplete.
 *
 * @param string $search_value
 *   The user search text.
 */
function cwrc_dtoc_edition_search_autocomplete($search_value) {
  global $user;
  module_load_include('inc', 'islandora_basic_collection', 'includes/utilities');
  $tuque = islandora_get_tuque_connection($user);
  $sparql_query = <<<EOQ
SELECT ?pid ?label
WHERE {
  ?pid <fedora-model:label> ?label ;
       <fedora-model:hasModel> <info:fedora/cwrc:dtocCModel> .
  FILTER(regex(?label, '$search_value', 'i') || regex(str(?pid), '$search_value', 'i'))
}
EOQ;
  $results = $tuque->repository->ri->sparqlQuery($sparql_query);
  $return = array();
  foreach ($results as $objects) {
    $return[$objects['pid']['value']] = t('@label (@pid)', array(
      '@label' => $objects['label']['value'],
      '@pid' => $objects['pid']['value'],
    ));

    $object = islandora_object_load($objects['pid']['value']);
    // Checking DToC datastream presence as well as object access to allow a
    // dtoc edition object to show up in the autocomplete.
    $access = !empty($object['DTOC']) ? islandora_xacml_editor_islandora_object_access('administer islandora_xacml_editor', $object, $user) : FALSE;

    if (!$access) {
      unset($return[$objects['pid']['value']]);
    }
  }
  drupal_json_output($return);
}
