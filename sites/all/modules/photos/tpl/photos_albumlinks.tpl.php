<?php
/**
 * @file
 * Formats album links.
 */
?>
<?php if (isset($links['link']) || isset($links['count'])): ?>
  <div class="photos_album_links">
    <div class="photos_link_info">
      <?php if ($links['count']) print t('!cou images', array('!cou' => $links['count'])); ?>
      <?php print $links['link']; ?>
    </div>
  <div class="photos_link_sort">
    <span><?php print t('Sort by:'); ?></span>
    <?php print $links['sort']; ?>
  </div>
  <?php if (isset($links['limit'])): ?>
    <div class="photos_link_limit">
      <span><?php print t('Limit:'); ?></span>
      <?php print $links['limit']; ?>
    </div>
  <?php endif; ?>
  </div>
<?php endif; ?>
