<?php
/**
 * @file
 * Formats album list.
 */
?>
<div class="photos-album">
  <?php if ($node->album['url']): ?>
    <div class="album-left"><img src="<?php print $node->album['url']; ?>"></div>
    <div class="album-right">
  <?php endif; ?>
  <h2><?php print $node->titlelink; ?></h2>
  <div class="photos-meta"><?php print $node->name; ?><span class="photos-time"><?php print t('Published in ') . $node->time; ?></span><span class="photos-imgnum"></span><?php print t('A total of %num images', array('%num' => $node->album['count'])); ?></div>
  <?php print $node->teaser; ?>
  <?php if ($node->album['url']): ?>
    </div>
  <?php endif; ?>
  <div class="photos-editmenu"><?php print $node->albumlink; ?></div>
</div>
