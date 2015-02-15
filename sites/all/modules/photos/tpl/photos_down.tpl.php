<?php
/**
 * @file
 * Formats share image url page.
 */
?>
<?php if ($type == 'print'): ?>
<!DOCTYPE HTML>
<html lang="<?php print $GLOBALS['language']->language; ?>" xml:lang="<?php print $GLOBALS['language']->language; ?>">
<head>
  <title><?php print drupal_get_title(); ?></title>
  <?php print drupal_get_html_head(); ?>
  <?php print drupal_get_css(); ?>
  <?php print drupal_get_js('header'); ?>
</head>
<body class="photos-download-page">
<?php endif; ?>
<div class="photos_download_bucket">
  <div class="photos_album_float_right">
    <?php print $content['info']; ?>
  </div>
  <?php if (isset($content['back'])): ?>
    <?php print $content['back']; ?>
  <?php endif; ?>
</div>
<div class="photos_download_menu">
  <div class="photos_download_menu_t">
    <?php if (isset($content['link']['sizes'])): ?>
      <?php print $content['link']['sizes']; ?>
    <?php endif; ?>
    <?php if (isset($content['link']['exif'])): ?>
      <?php print $content['link']['exif']; ?>
    <?php endif; ?>
    <?php if (isset($content['link']['vote'])): ?>
      <?php print $content['link']['vote']; ?>
    <?php endif; ?>
  </div>
  <?php if (isset($content['sizes'])): ?>
    <div class="photos_download_menu_b"><ul>
      <?php
        unset($content['link']['sizes']);
        foreach ($content['link'] as $link) {
          print '<li>' . $link . '</li>';
        }
      ?>
    </ul></div>
  <?php endif; ?>
</div>
 
<?php if (isset($content['sharing_url'])): ?>
<div id="photos-image-sharing">
<input type="text" class="image-quote-link" value='<a title="<?php print $content['sharing_title']; ?>" href="<?php print $content['sharing_link']; ?>"><img src="<?php print $content['sharing_url']; ?>"></a>'/>
<input type="text" class="image-quote-link" value="<?php print $content['sharing_url']; ?>"/>
</div>
<?php endif; ?>
 
<div class="photos_download_view">
  <?php print $content['view']; ?>
</div>
<?php if ($type == 'print'): ?>
<?php print drupal_get_js('footer'); ?>
</body>
</html>
<?php endif; ?>
