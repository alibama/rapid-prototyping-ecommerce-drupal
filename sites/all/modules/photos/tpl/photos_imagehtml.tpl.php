<?php
/**
 * @file
 * Formats single image.
 */
?>
<?php if (!isset($image->href)): ?>
  <?php print $image->view; ?>
<?php else: ?>
  <div class="photos_imagehtml">
    <?php
      print l($image->view, $image->href, array(
        'html' => true
      ));
    ?>
    <?php if (isset($image->colorbox)): ?>
      <a class="photos_imagehtml_colorbox colorbox" rel="colorbox_<?php print $image->pid; ?>" href="<?php print _photos_l($image->uri, 'origianl'); ?>" title="<?php print $title; ?>">view original</a>
    <?php endif; ?>
  </div>
<?php endif; ?>
