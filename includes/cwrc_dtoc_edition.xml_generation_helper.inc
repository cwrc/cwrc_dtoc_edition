<?php

/**
 * @file
 * File to implement some basic functionality for xml generation from php.
 */

/**
 * Gets an xml from pre-formatted array.
 *
 * @param array $data
 *   The data.
 *
 * @return string
 *   The generated xml string.
 */
function cwrc_dtoc_edition_get_xml(array $data) {
  $doc = new \DOMDocument();
  $child = _cwrc_dtoc_edition_generate_xml_element($doc, $data);
  if ($child) {
    $doc->appendChild($child);
  }
  $doc->formatOutput = TRUE;
  return $doc->saveXML();
}

/**
 * Generates an xml element.
 *
 * @param \DOMDocument $dom
 *   The dom document.
 * @param array $data
 *   The data.
 *
 * @return bool|\DOMElement
 *   The element.
 */
function _cwrc_dtoc_edition_generate_xml_element(\DOMDocument $dom, array $data) {
  if (empty($data['name'])) {
    return FALSE;
  }

  // Create the element.
  $element_value = !empty($data['value']) ? $data['value'] : NULL;
  $element = $dom->createElement($data['name'], $element_value);

  // Add any attributes.
  if (!empty($data['attributes']) && is_array($data['attributes'])) {
    foreach ($data['attributes'] as $attribute_key => $attribute_value) {
      $element->setAttribute($attribute_key, $attribute_value);
    }
  }

  // Any other items in the data array should be child elements.
  foreach ($data as $data_key => $child_data) {
    if (!is_numeric($data_key)) {
      continue;
    }

    $child = _cwrc_dtoc_edition_generate_xml_element($dom, $child_data);
    if ($child) {
      $element->appendChild($child);
    }
  }

  return $element;
}
