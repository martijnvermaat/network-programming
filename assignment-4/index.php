<?php
include('conference-shared.php');
?>

<?= page_header() ?>

<h2>Paper Storage</h2>

<p><a href="<?= $BASECGI ?>papers.cgi">Stored papers</a></p>

<h2>Hotel Reservation</h2>

<p><a href="<?= $BASEPHP ?>hotellist.php">Available rooms</a></p>
<p><a href="<?= $BASEPHP ?>hotelbook.php">Book a room</a></p>
<p><a href="<?= $BASEPHP ?>participants.php">Registered participants</a></p>

<?= page_footer() ?>
