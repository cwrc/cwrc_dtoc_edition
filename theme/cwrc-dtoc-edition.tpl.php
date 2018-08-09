<?php

/**
 * @file
 * This is the template file for the object page for cwrc dtoc edition.
 *
 * @TODO: add documentation about file and available variables
 */
?>

<div class="cwrc-dtoc-edition-object islandora" vocab="http://schema.org/" prefix="dcterms: http://purl.org/dc/terms/">
  <div class="cwrc-dtoc-edition-content-wrapper clearfix">
    <?php if (isset($islandora_content)): ?>
      <div class="cwrc-dtoc-edition-content">
        <?php print $islandora_content; ?>
      </div>
    <?php endif; ?>
  </div>
</div>
