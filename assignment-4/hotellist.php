<?php


// Include some shared functionality
include('conference-shared.php');


$socket = connect_to_gateway($HOTEL_GATEWAY_ADDR, $HOTEL_GATEWAY_PORT);

$request = array('procedure'  => 'list',
                 'parameters' => array());

$response = send_request($socket, $request);

$list = parse_list_response($response);

if ($list['status'] != 0) {
    die(error_page("Could not read response from hotel service:\n".$list['message']));
}

$availabilities = $list['content'];


echo page_header('Available Rooms');

echo '<h2>Available Rooms</h2>';

if (count($availabilities) < 1) {

    echo '<p>Unfortunately, there are no rooms available at this time.</p>';

} else {

    echo '<dl>';
    foreach ($availabilities as $availability) {
        echo '<dt>Rooms of type '.htmlentities($availability['type']).'</dt>';
        echo '<dd>'.htmlentities($availability['number']).' rooms available at &euro; '.sprintf('%.2f', $availability['price']).' per room';
        echo ' (<a href="'.$BASEPHP.'hotelbook.php?type='.htmlentities(urlencode($availability['type'])).'">book now!</a>)';
        echo '</dd>';
    }
    echo '</dl>';

}

echo page_footer();


?>
