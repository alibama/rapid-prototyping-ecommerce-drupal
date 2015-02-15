version:7.x-2.x

INSTALL
-------

  *Depends on core image and file module(s).
  *Content type photos created automatically as primary album type.
  *Configure global settings: admin/config/media/photos
  *Update permissions.
  *For inline editing photo title and description:
    1. Install the Libraries API 2.x module.
    2. Save jquery.jeditable.js AND jquery.jeditable.mini.js from http://www.appelsiini.net/projects/jeditable.
    3. Add both files to sites/all/libraries/jeditable.
  *For colorbox integration:
    1. Install the Colorbox module.
    2. On the Colorbox module settings page check "Enable Colorbox load" in extra settings and save.


LOCKED AND PASSWORD PROTECTED GALLERIES
---------------------------------------

  *Please note that locked and password protected galleries will not protect the actual image url.
    - For truly private / locked galleries you must set up Drupal to use a private file system.


SEO
---

  *To help prevent bots from voting if anonymous voting is enabled and for
    better SEO you may want to add the following to your robots.txt file:

# Paths (clean URLs)
Disallow: /photos/image/*/vote/
Disallow: /photos/zoom/

# Paths (no clean URLs)
Disallow: /?q=photos/image/*/vote/
Disallow: /?q=photos/zoom/


UPGRADE
-------

  *Upgrade from D6 to D7.
  *If you are using the photos_access module in D6 delete the directory from your modules folder.
    The photos_access module is now included as a sub module in D7 photos.
  *Files will be transfered from old core files database table to new file_managed table.
  *Backup database! Backup files table before upgrade.
  *Database tables are renamed from x_* to photos_* (update any custom modules / code).
  *When upgrading from Drupal 6 to Drupal 7 your images will be rebuilt
    with the core image caching system. Warning: do not delete your old
    image directories, they could still be in use by your users external
    links and shared galleries, etc.
  *Update core image styles to meet your needs.
  *Update gallery settings to use new image sizes: admin/config/media/photos.


FEATURES
--------

  photos:
  *Create photo galleries.
  *Upload and manage images.
  *Comment on images.
  *Vote on images.
  *Integrates with core image styles.
  *Support for Drupal core private file system.

  photos_swfu
  *Upload multiple images with SWFUpload.

  photos_access
  *Lock albums.
  *Password protected albums.
  *Create list of users who can access certain albums.
  *Create list of users who can edit albums, collaborators.

CREDITS
-------

  photos search integration by:
  R o n a l d   B a r n e s
