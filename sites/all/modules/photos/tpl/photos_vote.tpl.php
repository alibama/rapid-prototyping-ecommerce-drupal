<?php
/**
 * @file
 * Formats image votes.
 */
?>
<div class="photos-votes">
  <?php if (isset($vote['access']) && $vote['access']): ?>
    <a href="<?php print $vote['up']['#href']; ?>" class="photos-vote-u photos-vote-up-u" title="<?php print $vote['up']['#title']; ?>"></a>
    <span class="photos-vote-sum"><?php print $vote['count']['#sum']; ?> / <?php print $vote['count']['#count']; ?></span>
    <a href="<?php print $vote['down']['#href']; ?>" class="photos-vote-u photos-vote-down-u" title="<?php print $vote['down']['#title']; ?>"></a>
  <?php else: ?>
      <?php if (isset($vote['up']['#href']) && $vote['up']['#href']): ?>
        <a href="<?php print $vote['up']['#href']; ?>" class="photos-vote photos-vote-up" title="<?php print $vote['up']['#title']; ?>" alt="<?php print $fid; ?>" rel="nofollow"></a>
      <?php else: ?>
        <span class="photos-vote photos-vote-up photos-vote-up-x" title = "<?php print $vote['up']['#title']; ?>"></span>
      <?php endif; ?>

      <?php if (!isset($vote['count']['#href'])): ?>
        <span class="photos-vote-sum photos-vote-sum_<?php print $fid; ?>"><?php print $vote['count']['#sum']; ?> / <?php print $vote['count']['#count']; ?></span>
      <?php else: ?>
        <a href="<?php print $vote['count']['#href']; ?>" title="<?php print $vote['count']['#title']; ?>"><span class="photos-vote-sum photos-vote-sum_<?php print $fid; ?>"><?php print $vote['count']['#sum']; ?> / <?php print $vote['count']['#count']; ?></span></a>
      <?php endif; ?>
      
      <?php if (isset($vote['down']['#href']) && $vote['down']['#href']): ?>
        <a href="<?php print $vote['down']['#href']; ?>" class="photos-vote photos-vote-down" title = "<?php print $vote['down']['#title']; ?>" alt="<?php print $fid; ?>" rel="nofollow"></a>
      <?php else: ?>
        <span class="photos-vote photos-vote-down photos-vote-down-x" title = "<?php print $vote['down']['#title']; ?>"></span>
      <?php endif; ?>
  <?php endif; ?>
</div>
