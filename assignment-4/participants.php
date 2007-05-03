<?php


// Include some shared functionality
include('conference-shared.php');


$socket = connect_to_gateway($HOTEL_GATEWAY_ADDR, $HOTEL_GATEWAY_PORT);

$request = array('procedure'  => 'guests',
                 'parameters' => array());

$list = send_request($socket, $request);

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
