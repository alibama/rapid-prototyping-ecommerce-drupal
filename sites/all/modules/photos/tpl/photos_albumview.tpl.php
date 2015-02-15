<?php
/**
 * @file
 * Formats album layout.
 */
?>
<?php if (isset($album['album_url']) || isset($album['node_edit_url'])): ?>
<div class="photos_album_menu">
  <?php if (isset($album['access']['edit']) && !empty($album['access']['edit'])): ?>
    <?php print $album['node_edit_url']; ?>
    <?php print $album['image_management_url']; ?>
  <?php endif; ?>
</div>
<?php endif; ?>
<?php if (isset($node->type) && $node->type == 'photos'): ?>
  <div class="photos_album_header">
    <?php if (isset($node->album['cover']['url'])): ?>
      <div class="photos_album_cover">
        <?php print $album['cover']; ?>
      </div>
    <?php endif; ?>
    <h2><?php print check_plain($node->title); ?></h2>
    <?php if (isset($node->teaser)): ?>
      <?php print $node->teaser; ?>
    <?php endif; ?>
    <div class="photos_album_right">
      <?php print t('!cou images', array('!cou' => $node->album['count'])); ?>
      <div class="submitted">
        <?php print t('Submitted by !name on !date', array('!name' => $node->name, '!date' => format_date($node->created))); ?>
      </div>
    </div>
  </div>
<?php endif; ?>

<?php print $album['links']; ?>
<?php if (isset($album['pager'])): ?>
  <?php print theme_pager($album['pager']); ?>
<?php endif; ?>
<?php
  foreach ($album['view'] as $view) {
    print $view;
  }
?>
<?php if (isset($album['pager'])): ?>
  <?php print theme_pager($album['pager']); ?>
<?php endif; ?>
