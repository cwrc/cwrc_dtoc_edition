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
  $form['#attached']['js'][] = $path . '/js/ctools.js';
  $form['#attached']['js'][] = $path . '/js/dtoc_reveal_form.js';
  $form['#attached']['css'][] = $path . '/css/ctools.css';
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
    '#weight' => -99,
  );

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

  // Selected Collections.
  $form['sources']['collections'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => 'cwrc-dtoc-edition-sources-collections',
    ),
    '#tree' => TRUE,
  );
  $form['sources']['collections'] += _cwrc_dtoc_edition_reveal_form_collections_subform($form_state, $datastream_data);

  // Configure corpus container.
  $form['corpus_configuration'] = array(
    '#type' => 'fieldset',
    '#title' => t('Configure corpus'),
    '#description' => t('If you leave the configuration fields blank, a default configuration will be applied. It will only work for source documents that are build on the <a href="@link" target="_blank" title="view the DToC document template">DToC Document Template</a>', array(
      '@link' => 'https://cwrc.ca/templates/DToC_Document_Template.xml',
    )),
    '#tree' => TRUE,
    '#required' => TRUE,
    '#collapsible' => FALSE,
    '#weight' => -98,
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
function _cwrc_dtoc_edition_reveal_form_collections_subform(array &$form_state, $datastream_data) {
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
      'wrapper' => 'cwrc-dtoc-edition-sources-collections',
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
    $index_documents = array();
    $selected_collection_items = $form_state['selected_collection'];
    foreach ($selected_collection_items as $selected_collection_item) {
      $collection = islandora_object_load($selected_collection_item['object_id']);
      if ($collection) {
        $pid = drupal_html_class($collection->id);
        $options = _cwrc_dtoc_edition_get_collection_pid_options($collection);
        $selected_document_keys = cwrc_dtoc_edition_get_selected_collection_files_from_session($collection->id);
        $index_documents += $selected_document_keys;
        $subform['items'][$pid] = _cwrc_dtoc_edition_reveal_form_collections_subform_collection_item($collection, $selected_document_keys, $options);
      }
    }

    $default_index = NULL;
    if (!empty($datastream_data['corpusConfiguration']['dtocIndexDocPid'])) {
      $default_index = $datastream_data['corpusConfiguration']['dtocIndexDocPid'];
    }

    if (!$default_index || !isset($index_documents[$default_index])) {
      $default_index = _cwrc_dtoc_edition_get_array_key_first($index_documents);
    }
    $subform['index_document'] = array(
      '#type' => 'fieldset',
      '#title' => t('Index document'),
      '#tree' => TRUE,
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      'pid' => array(
        '#title' => t('Please select an index document for this edition (optional)'),
        '#type' => 'radios',
        '#options' => $index_documents,
        '#default_value' => $default_index,
      ),
    );
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
          'wrapper' => 'cwrc-dtoc-edition-sources-collections',
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
      'wrapper' => 'cwrc-dtoc-edition-sources-collections',
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
  $values = !empty($form_state['values']) ? $form_state['values'] : [];
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
    '#required' => $is_custom_xpaths,
    '#states' => array(
      'required' => array(
        array(':input[name="corpus_configuration[custom][optin]"]' => array('checked' => TRUE)),
      ),
    ),
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

  // Validating corpus configurations.
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
    elseif (empty($options['xpaths']['xmldocuments'])) {
      form_set_error('corpus_configuration][custom][options][xpaths][xmldocuments', t('Please configure the xPath for your documents (which will form the "chapters" of your DToC edition).'));
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
    $object_id = $object->id;
  }
  else {
    $object_id = $form_state['islandora_object_id'];
  }
  $values = $form_state['values'];
  $collections = $values['sources']['collections']['items'];
  $corpus_configuration = !empty($values['corpus_configuration']) ? $values['corpus_configuration'] : array();

  if (!empty($collections)) {
    $form_state['cwrc_dtoc_edition_reveal_ingested_object_id'] = $object_id;
    $form_state['cwrc_dtoc_edition_reveal_ingested_data']['collections'] = $collections;
    $form_state['cwrc_dtoc_edition_reveal_ingested_data']['corpus_configuration'] = $corpus_configuration;
    $form_state['cwrc_dtoc_edition_reveal_ingested_data']['corpus_configuration']['index_document_pid'] = $values['sources']['collections']['index_document']['pid'];
  }

  // Calling the submit batch from here in case this is an edit.
  if (!empty($form_state['edit_session_populated']) && !empty($form_state['islandora_object_id'])) {
    $form_state['redirect'] = "islandora/object/{$object_id}";
    drupal_set_message(t('DToC datastream has been updated'));
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
  return $form['sources']['collections'];
}

/**
 * Helper functions.
 */

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
  $data = _cwrc_dtoc_edition_xml_content_to_array($datastream->content);
  if (!empty($data['sources'])) {
    $collections = empty($data['sources']['collection']['@attributes']) && count($data['sources']['collection']) > 1 ? $data['sources']['collection'] : $data['sources'];
    foreach ($collections as $collection) {
      $collection_pid = $collection['@attributes']['pid'];
      $selected_files = array();
      if (!empty($collection['document']['@attributes']['pid'])) {
        // This means only one document/object was selected for this collection.
        $document_pid = $collection['document']['@attributes']['pid'];
        $document_label = $collection['document']['@attributes']['label'];
        $selected_files[$document_pid] = $document_label;
      }
      else {
        $documents = !empty($collection['document']) ? $collection['document'] : [];
        foreach ($documents as $document) {
          $selected_files[$document['@attributes']['pid']] = $document['@attributes']['label'];
        }
      }

      $form_state['selected_collection'][$collection_pid]['object_id'] = $collection_pid;
      cwrc_dtoc_edition_set_selected_collection_files_in_session($collection_pid, $selected_files);
    }
  }
  return $data;
}
