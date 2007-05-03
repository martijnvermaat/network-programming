<?php


// Include some shared functionality
include('conference-shared.php');


$socket = connect_to_gateway($HOTEL_GATEWAY_ADDR, $HOTEL_GATEWAY_PORT);

$request = "guests\n\n";
$total_sent = 0;

while ($total_sent < strlen($request)) {
    if (FALSE === ($sent = @socket_write($socket, substr($request, $total_sent)))) {
        die(error_page("Could not send request to hotel service:\n".socket_strerror(socket_last_error())));
    }
    if ($sent == 0) {
        die(error_page('Could not send request to hotel service'));
    }
    $total_sent += $sent;
}

$response = '';

do {
    if (FALSE === ($read = @socket_read($socket, 255))) {
        die(error_page("Could not read response from hotel service:\n".socket_strerror(socket_last_error())));
    }
    $response .= $read;
} while ($read != '');

$list = parse_response($response);

if ($list['status'] != 0) {
    die(error_page("Could not read response from hotel service:\n".$list['message']));
}

$guests = $list['content'];


echo page_header('Registered Participants');

echo '<h2>Registered Participants</h2>';

if (count($guests) < 1) {

    echo '<p>There are no participants registered at this time.</p>';

} else {

    echo '<ul>';
    foreach ($guests as $guest) {
        echo '<li>'.htmlentities($guest).'</li>';
    }
    echo '</ul>';

}

echo page_footer();


?>
