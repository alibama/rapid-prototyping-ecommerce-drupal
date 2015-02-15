<?php
/**
 * @file
 * Formats default layout.
 */
?>
<div id="photos_default">
  <?php if (isset($content['user']['image'])): ?>
    <h2><?php print t('My latest images'); ?></h2>
    <?php print $content['user']['image']; ?>
  <?php endif; ?>
  <?php if (isset($content['user']['album'])): ?>
    <h2><?php print t('My latest albums'); ?></h2>
    <?php print $content['user']['album']; ?>
  <?php endif; ?>
  <?php if (isset($content['site']['image'])): ?>
    <h2><?php print t('Latest images'); ?></h2>
    <?php print $content['site']['image']; ?>
  <?php endif; ?>
  <?php if (isset($content['site']['album'])): ?>
    <h2><?php print t('Latest albums'); ?></h2>
    <?php print $content['site']['album']; ?>
  <?php endif; ?>
</div>
