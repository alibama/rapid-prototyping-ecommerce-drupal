(function ($) {
Drupal.behaviors.photosFormTeaser = {
  attach: function(context) {
    var radioVal = $('input:radio[name="album[teaser_display]"]:checked').val();
    if (radioVal < 2) {
      $('.form-item-album-teaser-viewnum').hide('slow');
    }
    else {
      $('.form-item-album-teaser-viewnum').show('slow');
    }
    $('input:radio[name="album[teaser_display]"]').change(function() {
      radioVal = $('input:radio[name="album[teaser_display]"]:checked').val();
      if (radioVal < 2) {
        $('.form-item-album-teaser-viewnum').hide('slow');
      }
      else {
        $('.form-item-album-teaser-viewnum').show('slow');
      }
    });
  }
};
})(jQuery);
