<?php

include('site.php');

echo '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"';
echo ' "http://www.w3.org/TR/html4/strict.dtd">';

?>

<html lang="en">

<head>
    <title>Conference Website</title>
</head>

<body>


<h1>Conference Website</h1>

<h2>Paper Storage</h2>

<p><a href="<?= $BASECGI ?>papers.cgi">Stored papers</a></p>

<h2>Hotel Reservation</h2>

<p><a href="<?= $BASEPHP ?>hotellist.php">Available rooms</a></p>
<p><a href="<?= $BASEPHP ?>hotelbook.php">Book a room</a></p>
<p><a href="<?= $BASEPHP ?>participants.php">Registered participants</a></p>


</body>

</html>
