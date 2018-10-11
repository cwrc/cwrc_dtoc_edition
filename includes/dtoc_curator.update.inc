<?php

/**
 * @file
 * Implements various ajax and ctools endpoints needed by the reveal form.
 */

/**
 * Ctools modal title callback for collection files selector.
 *
 * @param \IslandoraFedoraObject $object
 *   The id of the object collection.
 *
 * @return string
 *   The title.
 */
function cwrc_dtoc_edition_ctools_modal_curator_update_title(IslandoraFedoraObject $object) {
  return t('Updating curator id for: @pid', array('@pid' => $object->label));
}

/**
 * Menu callback for the ctools modal collection files selector.
 *
 * @param \IslandoraFedoraObject $object
 *   The islandora fedora collection object.
 * @param string $curator_id
 *   The curator id.
 * @param string $js
 *   The ctools js argument.
 *
 * @return array|mixed
 *   The form or the modal containing the form.
 */
function cwrc_dtoc_edition_ctools_modal_curator_update(\IslandoraFedoraObject $object, $curator_id = '', $js = NULL) {
  // Instantiating form state.
  $form_state = array(
    'title' => t('Would you like to save this curation information for your CWRC DToC Edition of "@label"?', array(
      '@label' => $object->label,
      '@id' => $curator_id,
    )),
    'islandora_object' => $object,
    'curator_id' => $curator_id,
    'ajax' => (bool) $js,
  );
  $form_id = 'cwrc_dtoc_edition_ctools_modal_curator_update_form';

  if (!$js) {
    $args = func_get_args();
    // Remove $form_id from the arguments.
    array_shift($args);
    $form_state['build_info']['args'] = $args;
    return drupal_build_form($form_id, $form_state);
  }

  // Add necessary ctools includes.
  ctools_include('modal');
  ctools_include('ajax');

  $commands = ctools_modal_form_wrapper($form_id, $form_state);
  if (!empty($form_state['executed'])) {
    // When form successfully submitted we return ajax commands by overwriting
    // the commands.
    $commands = array();

    // Dismissing the form to return to the main form.
    $commands[] = ctools_modal_command_dismiss();
    if (!empty($form_state['cancel'])) {
      // Since the user is cancelling the curator id update mark update the js
      // setting that we didn't process it.
      $settings['cwrc_dtoc_edition']['processed_curator_id'] = FALSE;
      $commands[] = ajax_command_settings($settings, TRUE);
    }
    else {
      $commands[] = ctools_ajax_command_reload();
    }
  }

  print ajax_render($commands);
  exit;
}

/**
 * Form for updating curator id.
 *
 * @param array $form
 *   The initial form structure.
 * @param array $form_state
 *   The form state.
 *
 * @return array
 *   The full form structure.
 */
function cwrc_dtoc_edition_ctools_modal_curator_update_form(array $form, array &$form_state) {
  /** @var \IslandoraFedoraObject $collection_object */
  $dtoc_object = !empty($form_state['islandora_object']) ? $form_state['islandora_object'] : NULL;
  $curator_id = !empty($form_state['curator_id']) ? $form_state['curator_id'] : '';
  if (!$dtoc_object || !$curator_id) {
    $form['no_object'] = array(
      '#type' => 'html_tag',
      '#tag' => 'h2',
      '#value' => t('No valid CWRC DToC Edition provided or Wrong curator id'),
    );
    return $form;
  }

  $form['dtoc_object_pid'] = array(
    '#type' => 'hidden',
    '#value' => $dtoc_object->id,
  );
  $form['curator_id'] = array(
    '#type' => 'item',
    '#title' => t('Curator ID:'),
    '#markup' => t('<span> @curator</span>', array('@curator' => $curator_id)),
  );

  $form['actions'] = array(
    '#type' => 'actions',
    'yes' => array(
      '#type' => 'submit',
      '#value' => t('Yes'),
    ),
    'no' => array(
      '#type' => 'submit',
      '#value' => t('No'),
    ),
  );

  return $form;
}

/**
 * Submit callback for cwrc_dtoc_edition_ctools_modal_curator_update_form().
 */
function cwrc_dtoc_edition_ctools_modal_curator_update_form_submit(array $form, array &$form_state) {
  $triggering_element = $form_state['triggering_element'];
  $form_state['cancel'] = $triggering_element['#value'] === t('No');
  $islandora_object = $form_state['islandora_object'];
  if (!$form_state['cancel'] && !empty($islandora_object['DTOC'])) {
    module_load_include('inc', 'cwrc_dtoc_edition', 'includes/cwrc_dtoc_edition.xml_generation_helper');
    $xml_corpus_configure_key = 2;
    $datastream = $islandora_object['DTOC'];
    $curator_id = $form_state['curator_id'];
    $datastream_xml_data = _cwrc_dtoc_edition_xml_content_to_array($datastream->content);
    $data = _cwrc_dtoc_edition_prepare_datastream_content_array_for_xml_generation($datastream_xml_data);
    if (!empty($data[2][$xml_corpus_configure_key][1]) && $data[2][$xml_corpus_configure_key][1]['name'] === 'curation') {
      $data[2][$xml_corpus_configure_key][1]['attributes']['id'] = $curator_id;
      $xml = cwrc_dtoc_edition_get_xml($data);
      $islandora_object['DTOC']->setContentFromString($xml);
      drupal_set_message(t('The curation was successfully updated'));
    }
  }
}
