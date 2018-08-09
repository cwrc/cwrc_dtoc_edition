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
function cwrc_dtoc_edition_ctools_modal_collection_files_selector_title($object) {
  return t('Files selector: @pid', array('@pid' => $object->label));
}

/**
 * Menu callback for the ctools modal collection files selector.
 *
 * @param \IslandoraFedoraObject $object
 *   The islandora fedora collection object.
 * @param null $js
 *   The ctools js argument.
 *
 * @return array|mixed
 *   The form or the modal containing the form.
 */
function cwrc_dtoc_edition_ctools_modal_collection_files_selector($object, $js = NULL) {
  $collection_pid = $object->id;
  $selected_files = cwrc_dtoc_edition_get_selected_collection_files_from_session($collection_pid);
  $form_state = array(
    'title' => cwrc_dtoc_edition_ctools_modal_collection_files_selector_title($object),
    'islandora_object' => $object,
    'ajax' => (bool) $js,
    'selected_files' => $selected_files,
  );
  $form_id = 'cwrc_dtoc_edition_ctools_modal_collection_files_selector_form';

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

  $output = ctools_modal_form_wrapper($form_id, $form_state);
  if (!empty($form_state['executed'])) {
    // When form successfully submitted we return ajax commands by overwriting
    // the output.
    $output = array();

    // Dismissing the form to return to the main form.
    $output[] = ctools_modal_command_dismiss();
  }

  print ajax_render($output);
  exit;
}

/**
 * The collection files selector form.
 *
 * @param array $form
 *   The form structure.
 * @param array $form_state
 *   The form state.
 *
 * @return array
 *   The form already built.
 */
function cwrc_dtoc_edition_ctools_modal_collection_files_selector_form($form, &$form_state) {
  /** @var \IslandoraFedoraObject $collection_object */
  $collection_object = !empty($form_state['islandora_object']) ? $form_state['islandora_object'] : NULL;
  if (!$collection_object) {
    $form['no_collection'] = array(
      '#type' => 'html_tag',
      '#tag' => 'h2',
      '#value' => t('No valid collection provided'),
    );
    return $form;
  }

  $form['collection_pid'] = array(
    '#type' => 'hidden',
    '#value' => $collection_object->id,
  );

  $collection_files = _cwrc_dtoc_edition_get_collection_pid_options($collection_object);
  if ($collection_files) {
    $default = !empty($form_state['selected_files']) ? $form_state['selected_files'] : array();
    $form['collection_files'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Collection files'),
      '#options' => $collection_files,
      '#default_value' => array_keys($default),
      '#required' => TRUE,
    );

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Select'),
    );
  }
  else {
    $form['no_valid_collection_files'] = array(
      '#type' => 'html_tag',
      '#tag' => 'h2',
      '#value' => t("Collection doesn't have any CWRC document cModel."),
    );
  }

  return $form;
}

/**
 * Validates the ctools modal collection files selector form.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state.
 */
function cwrc_dtoc_edition_ctools_modal_collection_files_selector_form_validate(array $form, array $form_state) {
  $values = $form_state['values'];
  if (empty($values['collection_pid'])) {
    form_set_error(NULL, t('No valid collection provided'));
  }

  $collection_files = !empty($values['collection_files']) ? $values['collection_files'] : array();
  if (!array_filter($collection_files)) {
    form_set_error('collection_files', t('No collection files selected'));
  }
}

/**
 * Submit callback for the ctools modal collection files selector form.
 *
 * @param array $form
 *   The form array.
 * @param array $form_state
 *   The form state.
 */
function cwrc_dtoc_edition_ctools_modal_collection_files_selector_form_submit(array $form, array &$form_state) {
  $values = $form_state['values'];
  $options = $form['collection_files']['#options'];
  $selected = array_filter($values['collection_files']);

  cwrc_dtoc_edition_set_selected_collection_files_in_session($values['collection_pid'], array_intersect_key($options, $selected));
}

function _cwrc_dtoc_edition_get_collapsiblock_js_settings($arguments) {
  // A fix to make sure that collapsiblock contrib module  is not breaking our
  // functionality.
  if (module_exists('collapsiblock')) {
    global $theme;
    $current_theme = $theme ? $theme : variable_get('theme_default', 'bartik');
    $theme_settings = variable_get(str_replace('/', '_', 'theme_' . $current_theme . '_settings'), array());
    $theme_settings = array_merge(collapsiblock_default_settings(), $theme_settings, array());
    $arguments['collapsiblock'] = array(
      'blocks' => array_change_key_case(variable_get('collapsiblock_settings', array()), CASE_LOWER),
      'default_state' => variable_get('collapsiblock_default_state', 1),
      'slide_type' => variable_get('collapsiblock_slide_type', 1),
      'slide_speed' => variable_get('collapsiblock_slide_speed', 200),
      'block_title' => $theme_settings['collapsiblock_title'],
      'block' => $theme_settings['collapsiblock_block'],
      'block_content' => $theme_settings['collapsiblock_content'],
    );
  }
  return $arguments;
}
