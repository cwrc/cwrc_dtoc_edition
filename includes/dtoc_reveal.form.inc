<?php

/**
 * @file
 * Holds the dtoc reveal form.
 */

/**
 * CWRC DToC Edition reveal form.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 * @param \AbstractFedoraObject|NULL $object
 *   The DToC object.
 * @param \AbstractFedoraDatastream|NULL $datastream
 *   The DToC datastream object.
 *
 * @return array
 *   The form structure.
 */
function cwrc_dtoc_edition_reveal_form(array $form, array &$form_state, AbstractFedoraObject $object = NULL, AbstractFedoraDatastream $datastream = NULL) {
  $path = drupal_get_path('module', 'cwrc_dtoc_edition');
  $datastream_data = array();

  $form['#attributes']['class'][] = 'islandora-ingest-form--cwrc-dtoc-reveal-form';
  // To allow form alter check.
  $form_state['cwrc_dtoc_edition_reveal_form'] = TRUE;
  // Adding needed ctools includes.
  _cwrc_dtoc_edition_reveal_form_add_ctools_includes();
  $form['#attached']['js'][] = $path . '/js/dtoc_reveal_form.js';
  $form['#attached']['css'][] = $path . '/css/dtoc_reveal_form.css';

  if ($object && $datastream) {
    module_load_include('inc', 'islandora', 'includes/breadcrumb');
    drupal_set_breadcrumb(islandora_get_breadcrumbs($object));
    if (empty($form_state['edit_session_populated'])) {
      $datastream_data = _cwrc_dtoc_edition_reveal_form_populate_session_from_datastream($datastream, $form_state);
      $form_state['edit_session_populated'] = TRUE;
      $form_state['islandora_object_id'] = $object->id;
    }
  }

  // Main source container.
  $form['sources'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => t('Select sources'),
    '#required' => TRUE,
    '#collapsible' => FALSE,
    '#weight' => 1,
  );
  $form['sources']['collections'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => 'cwrc-dtoc-edition-sources-collections',
    ),
    '#tree' => TRUE,
  );
  $form['sources']['collections'] += _cwrc_dtoc_edition_reveal_form_collections_subform($form_state);

  // Search collection sources container.
  $search_title = t('search CWRC collections');
  $form['sources']['search_collections'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
    'search' => array(
      '#type' => 'textfield',
      '#title' => $search_title,
      '#title_display' => 'invisible',
      '#attributes' => array(
        'placeholder' => $search_title,
      ),
      '#autocomplete_path' => 'islandora/basic_collection/find_collections_user',
      '#ajax' => array(
        'event' => 'autocompleteSelect',
        'callback' => 'cwrc_dtoc_edition_reveal_form_search_collections_actions_ajax_callback',
        'wrapper' => 'cwrc-dtoc-edition--reveal-form--search-collections--actions-wrapper',
        'effect' => 'fade',
      ),
    ),
    'actions' => _cwrc_dtoc_edition_reveal_form_get_search_collections_actions($form_state),
  );

  // Configure corpus container.
  $form['corpus_configuration'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
    '#title' => t('Configure corpus'),
    '#required' => TRUE,
    '#collapsible' => FALSE,
    '#weight' => 2,
  );

  // Copy from cwrc edition.
  $search_title = t('search all DToC CWRC edition');
  $user_settings = !empty($datastream_data['corpusConfiguration']['userSettings']) ? $datastream_data['corpusConfiguration']['userSettings'] : array();
  $user_settings_type = !empty($user_settings['@attributes']['type']) ? $user_settings['@attributes']['type'] : '';
  $user_settings_value = !empty($user_settings['@attributes']['value']) ? $user_settings['@attributes']['value'] : '';
  $is_from_cwrc_dtoc_edition = $user_settings_type === 'copy_cwrc_edition';
  $form['corpus_configuration']['copy_cwrc_edition'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
    'optin' => array(
      '#type' => 'checkbox',
      '#title' => t('Copy from a CWRC DToC edition'),
      '#states' => array(
        // Disabling the optin checkbox for DToC CWRC Edition if dtoc edition OR
        // custom configuration checked.
        'disabled' => array(
          array(':input[name="corpus_configuration[copy_dtoc_edition][optin]"]' => array('checked' => TRUE)),
          array(':input[name="corpus_configuration[custom][optin]"]' => array('checked' => TRUE)),
        ),
      ),
      '#default_value' => $is_from_cwrc_dtoc_edition,
    ),
    'search' => array(
      '#type' => 'textfield',
      '#title' => $search_title,
      '#title_display' => 'invisible',
      '#attributes' => array(
        'placeholder' => $search_title,
      ),
      '#states' => array(
        // Visible only if the optin above checked AND the other one's
        // un-checked.
        'visible' => array(
          ':input[name="corpus_configuration[copy_cwrc_edition][optin]"]' => array('checked' => TRUE),
          ':input[name="corpus_configuration[copy_dtoc_edition][optin]"]' => array('checked' => FALSE),
          ':input[name="corpus_configuration[custom][optin]"]' => array('checked' => FALSE),
        ),
      ),
      '#autocomplete_path' => 'dtoc-edition/search-autocomplete',
      '#default_value' => $user_settings_value,
    ),
  );

  // Copy from DToC edition.
  $search_title = t('full Voyant URL');
  $is_from_voyant_dtoc_edition = $user_settings_type === 'copy_dtoc_edition';
  $form['corpus_configuration']['copy_dtoc_edition'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
    'optin' => array(
      '#type' => 'checkbox',
      '#title' => t('Copy from any DToC edition'),
      '#states' => array(
        // Disabling the optin checkbox for DToC Edition if dtoc cwrc edition OR
        // custom configuration checked.
        'disabled' => array(
          array(':input[name="corpus_configuration[copy_cwrc_edition][optin]"]' => array('checked' => TRUE)),
          array(':input[name="corpus_configuration[custom][optin]"]' => array('checked' => TRUE)),
        ),
      ),
      '#default_value' => $is_from_voyant_dtoc_edition,
    ),
    'search' => array(
      '#type' => 'textfield',
      '#title' => $search_title,
      '#title_display' => 'invisible',
      '#attributes' => array(
        'placeholder' => $search_title,
      ),
      '#states' => array(
        // Visible only if the optin above checked AND the other one's
        // un-checked.
        'visible' => array(
          ':input[name="corpus_configuration[copy_dtoc_edition][optin]"]' => array('checked' => TRUE),
          ':input[name="corpus_configuration[copy_cwrc_edition][optin]"]' => array('checked' => FALSE),
          ':input[name="corpus_configuration[custom][optin]"]' => array('checked' => FALSE),
        ),
      ),
      '#default_value' => $user_settings_value,
    ),
  );

  $is_custom_xpaths = $user_settings_type === 'custom';
  $form['corpus_configuration']['custom'] = array(
    '#type' => 'container',
    '#tree' => TRUE,
    'optin' => array(
      '#type' => 'checkbox',
      '#title' => t('Configure'),
      '#states' => array(
        // Disabling the optin checkbox for custom if dtoc cwrc edition OR
        // dtoc cwrc edition checked.
        'disabled' => array(
          array(':input[name="corpus_configuration[copy_cwrc_edition][optin]"]' => array('checked' => TRUE)),
          array(':input[name="corpus_configuration[copy_dtoc_edition][optin]"]' => array('checked' => TRUE)),
        ),
      ),
      '#default_value' => $is_custom_xpaths,
    ),
    'options' => _cwrc_dtoc_edition_reveal_form_custom_corpus_configuration_subform($form_state, $is_custom_xpaths, $datastream_data),
  );

  $form['edit_action'] = array(
    '#type' => 'actions',
    '#access' => $object && $datastream,
    'save' => array(
      '#type' => 'submit',
      '#value' => t('Save'),
    ),
  );

  return $form;
}

/**
 * Main form structure helper functions.
 */

/**
 * Builds collections sub structure form.
 *
 * @param array $form_state
 *   The form state.
 *
 * @return array
 *   The sub form structure.
 */
function _cwrc_dtoc_edition_reveal_form_collections_subform(array &$form_state) {
  $subform = array();
  $collection_pid = !empty($form_state['updated_collection_pid']) ? $form_state['updated_collection_pid'] : '';

  $subform['updated_collection_pid'] = array(
    '#type' => 'hidden',
    '#attributes' => array(
      'id' => 'cwrc-dtoc-edition--reveal-form--updated-collection-pid',
    ),
    '#value' => $collection_pid,
  );

  $subform['updated_collection_auto_submit_trigger'] = array(
    '#type' => 'submit',
    '#value' => t('Update collection'),
    '#attributes' => array(
      'class' => array(
        'element-invisible',
        'cwrc-dtoc-edition--reveal-form--updated-collection-action-trigger',
      ),
    ),
    '#name' => 'reveal-form--updated-collection-auto-submit-trigger',
    '#validate' => array('cwrc_dtoc_edition_reveal_form_updated_collection_auto_submit_trigger_validate'),
    '#submit' => array('cwrc_dtoc_edition_reveal_form_updated_collection_auto_submit_trigger_submit'),
    '#limit_validation_errors' => array(
      array('sources', 'collections', 'updated_collection_pid'),
    ),
    '#ajax' => array(
      'callback' => 'cwrc_dtoc_edition_reveal_form_sources_collections_ajax_callback',
      'wrapper' => 'cwrc-dtoc-edition--reveal-form--sources-collections-items-wrapper',
      'effect' => 'fade',
    ),
  );

  $subform['items'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => 'cwrc-dtoc-edition--reveal-form--sources-collections-items-wrapper',
    ),
    '#tree' => TRUE,
  );
  if (!empty($form_state['selected_collection'])) {
    $selected_collection_items = $form_state['selected_collection'];
    foreach ($selected_collection_items as $selected_collection_item) {
      $collection = islandora_object_load($selected_collection_item['object_id']);
      if ($collection) {
        $pid = drupal_html_class($collection->id);
        $options = _cwrc_dtoc_edition_get_collection_pid_options($collection);
        $selected_document_keys = cwrc_dtoc_edition_get_selected_collection_files_from_session($collection->id);
        $subform['items'][$pid] = _cwrc_dtoc_edition_reveal_form_collections_subform_collection_item($collection, $selected_document_keys, $options);
      }
    }
  }

  return $subform;
}

/**
 * Builds the selected collection item form structure.
 *
 * @param \FedoraObject $collection_item
 *   The collection object.
 * @param array $document_keys
 *   The selected document object keys for the collection.
 * @param array $document_options
 *   The selected document object options for the collection.
 *
 * @return array
 *   The form sub structure.
 */
function _cwrc_dtoc_edition_reveal_form_collections_subform_collection_item(\FedoraObject $collection_item, array $document_keys, array $document_options) {
  $id = $collection_item->id;
  $label = $collection_item->label;
  $pid = drupal_html_class($id);
  $selected_document_count = count($document_keys);
  $available_document_count = count($document_options);
  $collection_count = $selected_document_count === $available_document_count ? t('all XML docs') : t('@selected out of @available XML docs', array(
    '@selected' => count($document_keys),
    '@available' => count($document_options),
  ));
  $edit_id = 'ctools-ajax-edit-cwrc-dtoc-edition-button-' . $pid;

  return array(
    '#type' => 'container',
    '#attributes' => array('class' => array('subform-collection-item')),
    '#tree' => TRUE,
    'link' => array(
      '#type' => 'link',
      '#title' => t('@label (@pid) - @count', array(
        '@label' => $label,
        '@pid' => $id,
        '@count' => $collection_count,
      )),
      '#href' => 'islandora/object/' . $id,
    ),
    'pid' => array(
      '#type' => 'value',
      '#value' => $id,
    ),
    'label' => array(
      '#type' => 'value',
      '#value' => $label,
    ),
    'selected_files' => array(
      '#type' => 'value',
      '#value' => $document_keys,
    ),
    'actions' => array(
      '#type' => 'actions',
      '#attributes' => array('class' => array('subform-collection-item-actions')),
      'remove' => array(
        '#type' => 'submit',
        '#value' => t('Remove'),
        '#collection_object' => $collection_item,
        '#name' => 'subform-collection-item-remove-' . $pid,
        '#submit' => array('cwrc_dtoc_edition_reveal_form_collections_remove_item'),
        '#ajax' => array(
          'callback' => 'cwrc_dtoc_edition_reveal_form_sources_collections_ajax_callback',
          'wrapper' => 'cwrc-dtoc-edition--reveal-form--sources-collections-items-wrapper',
          'effect' => 'fade',
        ),
        '#limit_validation_errors' => array(
          array('sources', 'collections', 'items'),
        ),
        '#attributes' => array(
          'class' => array('subform-collection-item-remove'),
        ),
      ),
      'edit_ctools_button_' . $pid => array(
        '#type' => 'button',
        '#value' => t('Edit selections'),
        '#attributes' => array(
          'class' => array(
            'ctools-use-modal',
            'ctools-modal-ctools-cwrc-dtoc-edition-style',
          ),
        ),
        '#name' => 'subform-collection-item-edit-selection-' . $pid,
        '#input' => FALSE,
        '#id' => $edit_id,
      ),
      'edit_ctools_button_url_' . $pid => array(
        '#type' => 'hidden',
        '#attributes' => array('class' => array($edit_id . '-url')),
        '#value' => url('dtoc-edition/files-selector/' . $id . '/nojs'),
      ),
    ),
  );
}

/**
 * Gets search collections actions.
 *
 * @param array $form_state
 *   The main form states.
 *
 * @return array
 *   The actions.
 */
function _cwrc_dtoc_edition_reveal_form_get_search_collections_actions(array &$form_state) {
  $title_attribute = t('select a collection to enable this button.');
  $values = !empty($form_state['values']['sources']['search_collections']) ? $form_state['values']['sources']['search_collections'] : array();
  $actions = array(
    '#type' => 'actions',
    '#attributes' => array(
      'id' => 'cwrc-dtoc-edition--reveal-form--search-collections--actions-wrapper',
    ),
  );
  $actions['add_full'] = array(
    '#type' => 'submit',
    '#value' => t('Add in full'),
    '#name' => 'reveal-form--search-collections-add-full',
    '#limit_validation_errors' => array(
      array('sources', 'search_collections', 'search'),
    ),
    '#attributes' => array(
      'disabled' => 'disabled',
      'title' => $title_attribute,
      'class' => array('reveal-form--search-collections-add-full'),
    ),
    '#ajax' => array(
      'callback' => 'cwrc_dtoc_edition_reveal_form_sources_collections_ajax_callback',
      'wrapper' => 'cwrc-dtoc-edition--reveal-form--sources-collections-items-wrapper',
    ),
    '#submit' => array('cwrc_dtoc_edition_reveal_form_sources_search_collections_add_full_submit'),
    '#validate' => array('cwrc_dtoc_edition_reveal_form_sources_search_collections_add_full_validate'),
  );
  $actions['select_files'] = array(
    '#type' => 'button',
    '#value' => t('Select files'),
    '#name' => 'reveal-form--search-collections-select-files',
    '#limit_validation_errors' => array(
      array('sources', 'search_collections', 'search'),
    ),
    '#attributes' => array(
      'class' => array('ctools-use-modal', 'ctools-modal-ctools-cwrc-dtoc-edition-style'),
      'disabled' => 'disabled',
      'title' => $title_attribute,
    ),
    '#input' => FALSE,
  );
  $actions['select_files_url'] = array(
    '#type' => 'hidden',
  );

  // User have selected a collection.
  if ($values && !empty($values['search'])) {
    unset(
      $actions['add_full']['#attributes']['disabled'],
      $actions['add_full']['#attributes']['title'],
      $actions['select_files']['#attributes']['disabled'],
      $actions['select_files']['#attributes']['title']
    );

    $collection_pid = $values['search'];
    $collection_pid_attribute = drupal_html_class($collection_pid);
    $actions['add_full']['#collection_object'] = islandora_object_load($collection_pid);
    $actions['select_files']['#id'] = 'ctools-ajax-cwrc-dtoc-edition-button-' . $collection_pid_attribute;
    $actions['select_files']['#name'] = 'reveal-form--search-collections-select-files-' . $collection_pid_attribute;
    $actions['select_files_url'] += array(
      '#attributes' => array('class' => array('ctools-ajax-cwrc-dtoc-edition-button-' . $collection_pid_attribute . '-url')),
      '#value' => url('dtoc-edition/files-selector/' . $collection_pid . '/nojs'),
    );
  }
  else {
    $actions['add_full']['#attributes']['disabled'] = 'disabled';
    $actions['add_full']['#attributes']['title'] = $title_attribute;
    $actions['select_files']['#attributes']['disabled'] = 'disabled';
    $actions['select_files']['#attributes']['title'] = $title_attribute;
  }

  return $actions;
}

/**
 * Provides the custom corpus configuration subform.
 *
 * @param array $form_state
 *   The form state.
 * @param bool $is_custom_xpaths
 *   Either the setup should be custom.
 * @param array $default_datastream
 *   The default datastream content.
 *
 * @return array
 *   The subform structure.
 */
function _cwrc_dtoc_edition_reveal_form_custom_corpus_configuration_subform(array &$form_state, $is_custom_xpaths = FALSE, array $default_datastream = array()) {
  $default_configuration = !empty($default_datastream['corpusConfiguration']) ? $default_datastream['corpusConfiguration'] : array();
  if ($is_custom_xpaths && !empty($default_configuration['xmlXpathExpressions'])) {
    $xml_xpath_expressions = $default_configuration['xmlXpathExpressions'];
    $xml_content = !empty($xml_xpath_expressions['xmlContentXpath']) ? $xml_xpath_expressions['xmlContentXpath'] : '';
    $xml_title = !empty($xml_xpath_expressions['xmlTitleXpath']) ? $xml_xpath_expressions['xmlTitleXpath'] : '';
    $xml_author = !empty($xml_xpath_expressions['xmlAuthorXpath']) ? $xml_xpath_expressions['xmlAuthorXpath'] : '';
    $xml_documents = !empty($xml_xpath_expressions['xmlDocumentsXpath']) ? $xml_xpath_expressions['xmlDocumentsXpath'] : '';
  }
  else {
    $xml_content = '';
    $xml_title = '';
    $xml_author = '';
    $xml_documents = '';
  }
  $subform = array(
    '#type' => 'container',
    '#tree' => TRUE,
    '#states' => array(
      // Visible only if the optin for the custom corpus configuration is
      // checked AND the other one's un-checked.
      'visible' => array(
        ':input[name="corpus_configuration[custom][optin]"]' => array('checked' => TRUE),
        ':input[name="corpus_configuration[copy_dtoc_edition][optin]"]' => array('checked' => FALSE),
        ':input[name="corpus_configuration[copy_cwrc_edition][optin]"]' => array('checked' => FALSE),
      ),
    ),
  );
  $subform['xpaths'] = array(
    '#type' => 'fieldset',
    '#title' => t('xPaths'),
    '#description' => t('Define XPath Expressions for any of the following:'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => !$is_custom_xpaths,
    '#require' => TRUE,
  );
  $subform['xpaths']['xmlcontent'] = array(
    '#type' => 'textfield',
    '#title' => t('Content'),
    '#default_value' => $xml_content,
  );
  $subform['xpaths']['xmltitle'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $xml_title,
  );
  $subform['xpaths']['xmlauthor'] = array(
    '#type' => 'textfield',
    '#title' => t('Author'),
    '#default_value' => $xml_author,
  );
  $subform['xpaths']['xmldocuments'] = array(
    '#type' => 'textfield',
    '#title' => t('Documents'),
    '#default_value' => $xml_documents,
  );

  $default_curation = !empty($default_configuration['userSettings']['curation']) ? $default_configuration['userSettings']['curation'] : array();
  $subform['curation'] = array(
    '#type' => 'fieldset',
    '#title' => t('Curation <span class="light">(optional)</span>'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $subform['curation']['id'] = array(
    '#type' => 'textfield',
    '#title' => t('curatorId'),
    '#suffix' => t('<div class="">OR</div>'),
    '#default_value' => !empty($default_curation['@attributes']['id']) ? $default_curation['@attributes']['id'] : '',
  );
  $subform['curation']['from'] = array(
    '#type' => 'textfield',
    '#title' => t('copy from'),
    '#suffix' => t('<div class="">OR</div>'),
    '#default_value' => !empty($default_curation['@attributes']['from']) ? $default_curation['@attributes']['from'] : '',
  );
  $subform['curation']['custom'] = array(
    '#type' => 'textarea',
    '#title' => t('Custom curation'),
    '#title_display' => 'invisible',
    '#attributes' => array(
      'placeholder' => t('Provide your own'),
    ),
  );

  return $subform;
}

/**
 * Validate handlers.
 */

/**
 * Validates the auto submit button for the updated collection pid input.
 */
function cwrc_dtoc_edition_reveal_form_updated_collection_auto_submit_trigger_validate($form, &$form_state) {
  $collection_pid = $form_state['values']['sources']['collections']['updated_collection_pid'];
  if (!$collection_pid || empty($form_state['input']['sources']['collections']['updated_collection_pid'])) {
    form_set_error(NULL, t("Couldn't find the updated collection ID."));
  }
}

/**
 * Validates the "add full" for the "search collection" input.
 */
function cwrc_dtoc_edition_reveal_form_sources_search_collections_add_full_validate($form, &$form_state) {
  $triggering_element = $form_state['triggering_element'];
  if (empty($triggering_element['#collection_object'])) {
    form_set_error('sources][search_collections][search', t('Invalid selected collection'));
    return;
  }

  $object = $triggering_element['#collection_object'];

  $options = _cwrc_dtoc_edition_get_collection_pid_options($object);
  if (!$options) {
    form_set_error('sources][search_collections][search', t('No cwrc documents found in this collection'));
  }
}

/**
 * Main form validate handler.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 */
function cwrc_dtoc_edition_reveal_form_validate(array $form, array &$form_state) {
  $values = $form_state['values'];
  if (empty($values['sources']['collections']['items'])) {
    form_set_error('sources][search_collections][search', t('No sources was selected'));
  }

  // Validating custom xpaths if user opted in.
  $is_custom_xpaths = !empty($values['corpus_configuration']['custom']['optin']);
  $is_from_cwrc_dtoc_edition = !empty($values['corpus_configuration']['copy_cwrc_edition']['optin']);
  $is_from_voyant_dtoc_edition = !empty($values['corpus_configuration']['copy_dtoc_edition']['optin']);
  if ($is_custom_xpaths) {
    $options = $values['corpus_configuration']['custom']['options'];
    $has_xpath = FALSE;
    foreach ($options['xpaths'] as $xpath_value) {
      if (!empty($xpath_value)) {
        $has_xpath = TRUE;
        break;
      }
    }
    if (!$has_xpath) {
      form_set_error('corpus_configuration][custom][options][xpaths', t('No xPath provided!'));
    }
  }
  elseif ($is_from_cwrc_dtoc_edition) {
    $pid = !empty($values['corpus_configuration']['copy_cwrc_edition']['search']) ? $values['corpus_configuration']['copy_cwrc_edition']['search'] : '';
    $dtoc_edition_object = islandora_object_load($pid);
    if (!$dtoc_edition_object || empty($dtoc_edition_object['DTOC'])) {
      form_set_error(
        'corpus_configuration][copy_cwrc_edition][search',
        t('Please, provide a valid CWRC DToC edition to copy from.')
      );
    }
  }
  elseif ($is_from_voyant_dtoc_edition) {
    $copy_dtoc_edition_field = 'corpus_configuration][copy_dtoc_edition][search';
    // Getting the url value.
    $url = !empty($values['corpus_configuration']['copy_dtoc_edition']['search']) ? $values['corpus_configuration']['copy_dtoc_edition']['search'] : '';
    // Check if url is valid voyant dtoc url.
    $is_valid_dtoc_url = strpos($url, '//voyant-tools.org/dtoc') !== FALSE;
    // Parse the url to check if corpus id is provided.
    $parsed_url = $is_valid_dtoc_url ? drupal_parse_url($url) : array();
    // Displaying the errors accordingly.
    $error_message = '';
    if (!$is_valid_dtoc_url) {
      $error_message = t('Please, provide a valid voyant URL! - @status', array(
        '@status' => $is_valid_dtoc_url ? 'true' : 'false',
      ));
    }
    elseif (empty($parsed_url['query']['corpus'])) {
      $error_message = t('No corpus id was found in the provided voyant URL!');
    }

    if ($error_message) {
      form_set_error($copy_dtoc_edition_field, $error_message);
    }
  }
  else {
    form_set_error('sources][search_collections][search', t('Currently testing xpaths configuration please provide them in order to submit the form.'));
  }
}

/**
 * Submit handlers.
 */

/**
 * For the updated collection auto submit trigger.
 */
function cwrc_dtoc_edition_reveal_form_updated_collection_auto_submit_trigger_submit($form, &$form_state) {
  $collection_pid = $form_state['values']['sources']['collections']['updated_collection_pid'];
  $collection_pid = $collection_pid ? $collection_pid : $form_state['input']['sources']['collections']['updated_collection_pid'];
  if ($object = islandora_object_load($collection_pid)) {
    $form_state['selected_collection'][$object->id]['object_id'] = $object->id;
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * For the remove collection item.
 */
function cwrc_dtoc_edition_reveal_form_collections_remove_item($form, &$form_state) {
  $triggering_element = $form_state['triggering_element'];
  $removed_object = $triggering_element['#collection_object'];
  unset($form_state['selected_collection'][$removed_object->id]);
  cwrc_dtoc_edition_delete_selected_collection_files_from_session($removed_object->id);
  $form_state['rebuild'] = TRUE;
}

/**
 * For the "add full" submit "search collection" input.
 */
function cwrc_dtoc_edition_reveal_form_sources_search_collections_add_full_submit($form, &$form_state) {
  $triggering_element = $form_state['triggering_element'];
  $object = $triggering_element['#collection_object'];
  $object_id = $object->id;
  $options = _cwrc_dtoc_edition_get_collection_pid_options($object);
  $form_state['selected_collection'][$object_id]['object_id'] = $object_id;
  cwrc_dtoc_edition_set_selected_collection_files_in_session($object_id, $options);
  $form_state['rebuild'] = TRUE;
}

/**
 * Main form submit handler.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   The form state.
 */
function cwrc_dtoc_edition_reveal_form_submit(array $form, array &$form_state) {
  if (empty($form_state['edit_session_populated'])) {
    module_load_include('inc', 'islandora', 'includes/ingest.form.inc');

    $object = islandora_ingest_form_get_object($form_state);
    if (!isset($object['DTOC'])) {
      $datastream = $object->constructDatastream('DTOC', 'M');
      $datastream->mimetype = 'application/xml';
      $object->ingestDatastream($datastream);
    }
  }
  else {
    $object = islandora_object_load($form_state['islandora_object_id']);
  }
  $values = $form_state['values'];
  $collections = $values['sources']['collections']['items'];
  $corpus_configuration = !empty($values['corpus_configuration']) ? $values['corpus_configuration'] : array();
  if (!empty($collections)) {
    $form_state['cwrc_dtoc_edition_reveal_ingested_object_id'] = $object->id;
    $form_state['cwrc_dtoc_edition_reveal_ingested_data']['collections'] = $collections;
    $form_state['cwrc_dtoc_edition_reveal_ingested_data']['corpus_configuration'] = $corpus_configuration;
  }

  // Calling the submit batch from here in case this is an edit.
  if (!empty($form_state['edit_session_populated']) && !empty($form_state['islandora_object_id'])) {
    cwrc_dtoc_edition_reveal_form_batch_submit($form, $form_state);
  }

  // Clearing the session.
  cwrc_dtoc_edition_delete_selected_collection_files_from_session();
}

/**
 * Ajax callbacks.
 */

/**
 * For the search collections input.
 */
function cwrc_dtoc_edition_reveal_form_search_collections_actions_ajax_callback($form, $form_state) {
  return $form['sources']['search_collections']['actions'];
}

/**
 * For sources collections.
 */
function cwrc_dtoc_edition_reveal_form_sources_collections_ajax_callback($form, $form_state) {
  return $form['sources']['collections']['items'];
}

/**
 * Batch helpers functions.
 */

/**
 * Prepares the selected collections and their documents to be inserted into data.
 *
 * @param array $data
 *   The data to be stored in xml.
 * @param array $collections
 *   Selected collections.
 * @param array $context
 *   The batch context.
 */
function cwrc_dtoc_edition_reveal_form_batch_prepare_collections_source_xml_data(array $data, array $collections, array &$context) {
  $collection_counter = 1;
  $context['message'] = t('Preparing xml datastream data for selected collections');
  foreach ($collections as $collection) {
    $selected_documents = $collection['selected_files'];
    unset($collection['selected_files'], $collection['actions']);
    $data[1][$collection_counter] = array(
      'name' => 'collection',
      'attributes' => $collection,
    );
    foreach ($selected_documents as $object_id => $object_label) {
      $data[1][$collection_counter][] = array(
        'name' => 'document',
        'attributes' => array('pid' => $object_id, 'label' => $object_label),
      );
    }
    $collection_counter++;
  }
  $context['results']['xml_data'] = $data;
}

function cwrc_dtoc_edition_reveal_form_batch_prepare_collection_items_datastream_files(array $collection_values, &$context) {
  $pid = $collection_values['pid'];
  $label = $collection_values['label'];
  $selected_documents = $collection_values['selected_files'];
  $context['message'] = t('Generating xml datastream file for @label (@id)', array(
    '@label' => $label,
    '@id' => $pid,
  ));

  $collection_files = _cwrc_dtoc_edition_reveal_form_get_collection_selected_documents_files($pid, $selected_documents);
  $context['results']['dtoc_input_files'][$pid] = $collection_files;
  $files_url_query = !empty($context['results']['dtoc_input_files_query']) ? $context['results']['dtoc_input_files_query'] : '';
  foreach ($collection_files as $file) {
    $files_url_query .= $file['url'] . '%0A';
  }
  $context['results']['dtoc_input_files_query'] = $files_url_query;
}

function cwrc_dtoc_edition_reveal_form_batch_get_user_corpus_configurations(array $corpus_configurations, &$context) {
  $xpaths = array();
  $context['results']['xml_xpaths_setup'] = array();
  $is_from_cwrc_dtoc_edition = !empty($corpus_configurations['copy_cwrc_edition']['optin']);
  $is_from_voyant_dtoc_edition = !empty($corpus_configurations['copy_dtoc_edition']['optin']);
  $is_custom_xpaths = !empty($corpus_configurations['custom']['optin']);
  $supported_xpath = array(
    'xmlAuthorXpath',
    'xmlContentXpath',
    'xmlDocumentsXpath',
    'xmlTitleXpath',
  );
  $context['message'] = t('Getting xPaths customization from corpus configurations...');

  if ($is_from_cwrc_dtoc_edition && !empty($corpus_configurations['copy_cwrc_edition']['search'])) {
    $pid = $corpus_configurations['copy_cwrc_edition']['search'];
    if (($dtoc_edition_object = islandora_object_load($pid)) && !empty($dtoc_edition_object['DTOC'])) {
      $xml_arr = (array) simplexml_load_string($dtoc_edition_object['DTOC']->content);
      $data = drupal_json_decode(drupal_json_encode($xml_arr));
      $xpaths = !empty($data['corpusConfiguration']['xmlXpathExpressions']) ? $data['corpusConfiguration']['xmlXpathExpressions'] : array();
    }
    $context['results']['xml_xpaths_setup']['type'] = 'copy_cwrc_edition';
    $context['results']['xml_xpaths_setup']['value'] = $pid;
  }
  elseif ($is_from_voyant_dtoc_edition && !empty($corpus_configurations['copy_dtoc_edition']['search'])) {
    $url_arr = drupal_parse_url($corpus_configurations['copy_dtoc_edition']['search']);
    $corpus = $url_arr['query']['corpus'];
    $tool = 'corpus.DocumentsMetadata';
    $path = 'https://voyant-tools.org/trombone';
    $query = array(
      'corpus' => $corpus,
      'tool' => $tool,
    );
    $url = url($path, array('query' => $query));
    $request = drupal_http_request($url);
    if ($request->code === '200') {
      $request_data = drupal_json_decode($request->data);
      if (!empty($request_data['documentsMetadata']['total']) && $request_data['documentsMetadata']['total'] >= 1) {
        $query_parameters = $request_data['documentsMetadata']['documents'][0]['parent_queryParameters'];
        $query_parameters = decode_entities($query_parameters);
        parse_str($query_parameters, $data);
        $xpaths = array_intersect_key($data, drupal_map_assoc($supported_xpath));
      }
    }
    $context['results']['xml_xpaths_setup']['type'] = 'copy_dtoc_edition';
    $context['results']['xml_xpaths_setup']['value'] = $corpus_configurations['copy_dtoc_edition']['search'];
  }
  elseif ($is_custom_xpaths && !empty($corpus_configurations['custom']['options']['xpaths'])) {
    $custom_xpaths = $corpus_configurations['custom']['options']['xpaths'];
    foreach ($supported_xpath as $xpath_key) {
      $xpath_key_form = strtolower(substr($xpath_key, 0, -5));
      if (!empty($custom_xpaths[$xpath_key_form])) {
        $xpaths[$xpath_key] = $custom_xpaths[$xpath_key_form];
      }
    }
    $context['results']['xml_xpaths_setup']['type'] = 'custom';
    $context['results']['xml_xpaths_setup']['curation'] = $corpus_configurations['custom']['options']['curation'];
  }
  $context['results']['xml_xpaths'] = $xpaths;
}

function cwrc_dtoc_edition_reveal_form_batch_get_corpus_metadata($pid, &$context) {
  $input = !empty($context['results']['dtoc_input_files_query']) ? $context['results']['dtoc_input_files_query'] : '';
  $xpaths = $context['results']['xml_xpaths'];
  $input_format = '';
  $trombone_data = array();
  $context['message'] = t('Getting corpus metadata from Voyant...');

  if ($input) {
    $target_url = 'https://voyant-tools.org/trombone';
    $input = substr($input, 0, -3);
    $input = str_replace(array('%3A', '.xml%250A'), array(':', '.xml%0A'), $input);

    // Initializing default parameters.
    $parameters = array(
      'palette' => 'default',
      'inkeTags' => 'false',
      'suppressTools' => 'false',
      'input' => $input,
      'tool' => 'corpus.CorpusMetadata',
    );
    if ($input_format) {
      $parameters['inputFormat'] = $input_format;
    }
    // Adding xPath user options.
    foreach ($xpaths as $key => $option) {
      $parameters[$key] = $option;
    }

    // Adding title and subTitle based MODS datastream.
    if (($object = islandora_object_load($pid)) && !empty($object['MODS'])) {
      $mods_datastream = $object['MODS'];
      $mods_data = drupal_json_decode(drupal_json_encode((array) simplexml_load_string($mods_datastream->content)));
      if (!empty($mods_data['titleInfo']['title'])) {
        $parameters['title'] = $mods_data['titleInfo']['title'];
      }

      if (!empty($mods_data['titleInfo']['subTitle'])) {
        $parameters['subTitle'] = $mods_data['titleInfo']['subTitle'];
      }
    }

    $data = drupal_http_build_query($parameters);
    $data = str_replace(array('%3A', '.xml%250A'), array(':', '.xml%0A'), $data);
    $url = url($target_url);
    $request_options = array(
      'headers' => array(
        'Content-Type' => 'application/x-www-form-urlencoded; charset=UTF-8',
      ),
      'method' => 'POST',
      'data' => $data,
    );
    $result = drupal_http_request($url, $request_options);
    if ($result->code != '200') {
      $code = $result->code;
      $message = '<p>Error code: %code<br>Error message: %error<br>Query: %query</p><p>%data</p>';
      $variables = array(
        '%code' => $code,
        '%error' => $result->error,
        '%query' => $data,
        '%data' => $code != -1 ? $result->data : '<none>',
      );
      watchdog(
        'cwrc_dtoc_edition',
        $message,
        $variables,
        WATCHDOG_WARNING,
        current_path()
      );
      drupal_set_message(t('An error occurred during the communication with Voyant. Try again later.'), 'error');
    }
    else {
      $message = '<p>Request code: %code<br>Query: %query</p><p>%data</p>';
      $variables = array(
        '%code' => $result->code,
        '%query' => $data,
        '%data' => $result->data,
      );
      watchdog(
        'cwrc_dtoc_edition',
        $message,
        $variables,
        WATCHDOG_NOTICE
      );
      $trombone_data = drupal_json_decode($result->data);
    }
  }
  $context['results']['trombone_data'] = $trombone_data;
}

function cwrc_dtoc_edition_reveal_form_batch_delete_datastream_files(array $collection_values, &$context) {
  $pid = $collection_values['pid'];
  $label = $collection_values['label'];
  $context['message'] = t('Deleting generated xml datastream file for @label (@id)', array(
    '@label' => $label,
    '@id' => $pid,
  ));
  $files_by_collection = !empty($context['results']['dtoc_input_files'][$pid]) ? $context['results']['dtoc_input_files'][$pid] : array();
  foreach ($files_by_collection as $file) {
    file_unmanaged_delete($file['uri']);
  }
}

function cwrc_dtoc_edition_reveal_form_batch_prepare_corpus_configuration_xml_data(&$context) {
  $xml_corpus_id_key = 1;
  $xml_corpus_configure_key = 2;
  $xml_xpaths_key = 3;
  $xml_trombone_key = 4;
  $trombone_data = !empty($context['results']['trombone_data']) ? $context['results']['trombone_data'] : array();
  $data = !empty($context['results']['xml_data']) ? $context['results']['xml_data'] : array();
  $xml_xpaths = !empty($context['results']['xml_xpaths']) ? $context['results']['xml_xpaths'] : array();
  $context['message'] = t('Preparing xml datastream data for corpus configuration and metadata from Voyant...');

  // Adding corpus metadata to our xml.
  if ($trombone_data && !empty($trombone_data['corpus']['metadata']['id'])) {
    $corpus_metadata = $trombone_data['corpus']['metadata'];

    // Setting up the corpus id.
    $data[2][$xml_corpus_id_key] = array(
      'name' => 'id',
      'value' => $corpus_metadata['id'],
    );

    // Setting up other trombone stuff.
    $data[2][$xml_trombone_key] = array(
      'name' => 'trombone',
      'attributes' => array(),
    );
    // Adding trombone data as attributes.
    foreach ($trombone_data as $data_name => $data_value) {
      if ($data_name !== 'corpus' && (is_string($data_value) || is_numeric($data_value))) {
        $data[2][$xml_trombone_key]['attributes'][$data_name] = $data_value;
      }
    }
    // Adding corpus metadata from trombone.
    $data[2][$xml_trombone_key][1] = array(
      'name' => 'CorpusMetadata',
    );
    $xml_data_counter = 1;
    foreach ($corpus_metadata as $metadata_label => $metadata_value) {
      if ($metadata_label !== 'id' && (is_string($metadata_value) || is_numeric($metadata_value))) {
        $data[2][$xml_trombone_key][1][$xml_data_counter] = array(
          'name' => $metadata_label,
          'value' => $metadata_value,
        );
        $xml_data_counter++;
      }
      elseif ($metadata_label === 'languageCodes' && is_array($metadata_value)) {
        // Dealing with language codes which is an array.
        $data[2][$xml_trombone_key][1][$xml_data_counter] = array(
          'name' => $metadata_label,
        );
        foreach ($metadata_value as $language_code) {
          $data[2][$xml_trombone_key][1][$xml_data_counter][] = array(
            'name' => 'language',
            'value' => $language_code,
          );
        }
        $xml_data_counter++;
      }
    }
  }

  // Adding the way xml xpaths configurations was applied.
  $corpus_configuration_setup = $context['results']['xml_xpaths_setup'];
  if (!empty($corpus_configuration_setup['type'])) {
    $data[2][$xml_corpus_configure_key] = array(
      'name' => 'userSettings',
      'attributes' => array('type' => $corpus_configuration_setup['type']),
    );
    $type = $corpus_configuration_setup['type'];
    if ($type === 'copy_dtoc_edition' || $type === 'copy_cwrc_edition') {
      $data[2][$xml_corpus_configure_key]['attributes']['value'] = $corpus_configuration_setup['value'];
    }
    else {
      $curation = $corpus_configuration_setup['curation'];
      $data[2][$xml_corpus_configure_key][1] = array(
        'name' => 'curation',
        'attributes' => array(
          'id' => !empty($curation['id']) ? $curation['id'] : '',
          'from' => !empty($curation['from']) ? $curation['from'] : '',
        ),
        'value' => !empty($curation['custom']) ? $curation['custom'] : '',
      );
    }
  }

  // Adding custom xml configuration provided.
  $xml_data_counter = 1;
  $data[2][$xml_xpaths_key] = array('name' => 'xmlXpathExpressions');
  foreach ($xml_xpaths as $xml_xpath_label => $xml_xpath) {
    $data[2][$xml_xpaths_key][$xml_data_counter] = array(
      'name' => $xml_xpath_label,
      'value' => $xml_xpath,
    );
    $xml_data_counter++;
  }

  ksort($data[2]);
  // @todo add corpus configuration from the form.
  $context['results']['xml_data'] = $data;
}

function cwrc_dtoc_edition_reveal_form_batch_generate_datastream_content($pid, &$context) {
  $data = $context['results']['xml_data'];
  $context['message'] = t('Creating the DToC datastream...');
  if ($object = islandora_object_load($pid)) {
    $dsid = 'DTOC';
    $datastream = isset($object[$dsid]) ? $object[$dsid] : $object->constructDatastream($dsid, 'M');

    module_load_include('inc', 'cwrc_dtoc_edition', 'includes/cwrc_dtoc_edition.xml_generation_helper');
    $xml = cwrc_dtoc_edition_get_xml($data);

    $datastream->label = $dsid;
    $datastream->mimetype = 'application/xml';
    $datastream->setContentFromString($xml);

    // Ingest the datastream if it isn't yet.
    if (!isset($object[$dsid])) {
      $object->ingestDatastream($datastream);
    }
  }
  else {
    watchdog('cwrc_dtoc_edition', 'was unable to load the object with PID: @pid - given', array('@pid' => $pid), WATCHDOG_ERROR);
  }
}

/**
 * Helper functions.
 */

/**
 * Helper function to add all the ctools needed includes.
 */
function _cwrc_dtoc_edition_reveal_form_add_ctools_includes() {
  // Include the CTools tools that we need.
  ctools_include('ajax');
  ctools_include('modal');

  // Add CTools' javascript to the page.
  ctools_modal_add_js();

  // Add our own style.
  $ctools_style = _cwrc_dtoc_edition_reveal_form_get_ctools_form_js_style();
  drupal_add_js($ctools_style, 'setting');
}

/**
 * Gets custom ctools js style.
 *
 * @return array
 *   Of custom styles.
 *
 * @throws \Exception
 */
function _cwrc_dtoc_edition_reveal_form_get_ctools_form_js_style() {
  // Create our own javascript that will be used to theme a modal.
  return array(
    'ctools-cwrc-dtoc-edition-style' => array(
      'modalSize' => array(
        'type' => 'fixed',
        'width' => 500,
        'height' => 300,
        'addWidth' => 20,
        'addHeight' => 15,
      ),
      'modalOptions' => array(
        'opacity' => .5,
        'background-color' => '#000',
      ),
      'animation' => 'fadeIn',
      'modalTheme' => 'CToolsCWRCDtocEditionModal',
      'throbber' => theme('image', array(
        'path' => ctools_image_path('ajax-loader.gif', 'cwrc_dtoc_edition'),
        'alt' => t('Loading...'),
        'title' => t('Loading'),
      )),
    ),
  );
}

/**
 * Gets the collection selected sources files.
 *
 * @param string $collection_pid
 *   The collection pid.
 * @param array $selected_documents
 *   The selected documents.
 *
 * @return array
 *   The documents files.
 */
function _cwrc_dtoc_edition_reveal_form_get_collection_selected_documents_files($collection_pid, array $selected_documents) {
  module_load_include('inc', 'islandora_cwrc_writer', 'includes/utilities');
  $filters = array(
    ' ' => '_',
    '-' => '_',
    '/' => '_',
    '[' => '_',
    ']' => '_',
    ':' => '_',
  );
  $id = strtolower($collection_pid);
  $id = drupal_clean_css_identifier($id, $filters);
  $random_string = substr(md5(rand()), 0, 7);
  $collection_files = array();
  foreach ($selected_documents as $object_id => $object_name) {
    if (($object = islandora_object_load($object_id)) && isset($object['CWRC'])) {
      $datastream = $object['CWRC'];
      $schema = islandora_cwrc_writer_get_document_schema_object($object);
      $schema_mapping_id = $schema ? islandora_cwrc_writer_get_schema_mapping($schema) : '';
      $schema_mapping = drupal_clean_css_identifier(strtolower($schema_mapping_id), $filters);
      $datastream_id = drupal_clean_css_identifier(strtolower($datastream->id), $filters);
      $sub_folder = $datastream_id . '_' . $schema_mapping;
      $path = 'public://cwrc_dtoc_edition/' . $sub_folder . '/';
      $path .= $id . '/' . drupal_clean_css_identifier(strtolower($object_id), $filters) . '_' . $random_string;

      $local_uri = file_stream_wrapper_uri_normalize($path . '.xml');
      $dirname = drupal_dirname($local_uri);
      file_prepare_directory($dirname, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);
      file_unmanaged_save_data($datastream->content, $local_uri);

      $collection_files[$object_id] = array(
        'uri' => $local_uri,
        'url' => file_create_url($local_uri),
        'schema_id' => $schema_mapping_id,
      );
    }
  }

  return $collection_files;
}

/**
 * Populates user session with current datastream collections.
 *
 * @param \AbstractFedoraDatastream $datastream
 *   The CWRC DToC edition datastream.
 * @param array $form_state
 *   The current form state.
 *
 * @return array
 *   The datastream content as array.
 */
function _cwrc_dtoc_edition_reveal_form_populate_session_from_datastream(AbstractFedoraDatastream $datastream, array &$form_state) {
  $data = drupal_json_decode(drupal_json_encode((array) simplexml_load_string($datastream->content)));
  if (!empty($data['sources'])) {
    $collections = empty($data['sources']['collection']['@attributes']) && count($data['sources']['collection']) > 1 ? $data['sources']['collection'] : $data['sources'];
    foreach ($collections as $collection) {
      $collection_pid = $collection['@attributes']['pid'];
      $documents = !empty($collection['document']) ? $collection['document'] : array();
      $selected_files = array();
      foreach ($documents as $document) {
        $selected_files[$document['@attributes']['pid']] = $document['@attributes']['label'];
      }

      $form_state['selected_collection'][$collection_pid]['object_id'] = $collection_pid;
      cwrc_dtoc_edition_set_selected_collection_files_in_session($collection_pid, $selected_files);
    }
  }
  return $data;
}
