<?php

include('site.php');

echo '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"';
echo ' "http://www.w3.org/TR/html4/strict.dtd">';

?>

<html lang="en">

<head>
    <title>Conference Website - Available Rooms</title>
</head>

<body>


<h1>Conference Website - Available Rooms</h1>

<p><a href="<?= $BASEPHP ?>index.php">Back to Conference Website</a></p>

<h2>Available Rooms</h2>


<?php

// TODO: error handling

$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);

socket_bind($socket, $_SERVER['SERVER_ADDR']);

socket_connect($socket, $HOTEL_GATEWAY_ADDR, $HOTEL_GATEWAY_PORT);

socket_write($socket, "list\n\n");

echo '<pre>'.socket_read($socket, 255).'</pre>';

?>


</body>

</html>
