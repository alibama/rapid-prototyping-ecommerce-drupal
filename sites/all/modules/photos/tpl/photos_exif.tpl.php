<?php
/**
 * @file
 * Formats exif data.
 */
?>
<?php if (is_array($exif)): ?>
<table>
  <?php foreach ($exif as $key => $tag) { ?>
      <?php if (is_array($tag)): ?>
        <tr class="photos_exif_title"><td colspan="2"><?php print $key; ?></td></tr>
        <?php foreach ($tag as $ctag => $val) { ?>
          <tr><td class="photos_exif_name" width="30%"><?php print $ctag; ?></td><td>
          <?php if (!is_array($val)): ?>
            <?php print $val; ?>
          <?php else: ?>
            <?php print implode(',', $val); ?>
          <?php endif; ?>
          </td></tr>
        <?php } ?>
      <?php endif; ?>
    <?php } ?>
    </table>
<?php else: ?>
  <h2><?php print $exif; ?></h2>
<?php endif; ?>
