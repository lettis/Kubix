<?php

// file names
$client_list_filename = "kubix_clients.dat";

//TODO: switch from file mechanism to sql-db.

function load($file) {
  if (file_exists($file)) {
    $fh = fopen($file, 'r');
    $dat = fread($fh, filesize($file));
    fclose($fh);
    return unserialize($dat);
  } else {
    return [];
  }
}

function save($file, $obj) {
  $fh = fopen($file, 'w');
  fwrite($fh, serialize($obj));
  fclose($fh);
}





function check_client($client_id, $client_pw) {
  global $client_list_filename;

  if ($client_id == "") {
    return false;
  }
  $client_list = load($client_list_filename);
  if ($client_list[$client_id] == password_hash($client_pw, PASSWORD_BCRYPT)) {
    return true;
  }
  return false;
}


function register_client($client_id, $client_pw) {
  global $client_list_filename;

  $client_list = load($client_list_filename);
  if (array_key_exists($client_id, $client_list)) {
    return false;
  } else {
    $client_list[$client_id] = password_hash($client_pw, PASSWORD_BCRYPT);
    save($client_list_filename, $client_list);
    return true;
  }
}

function is_valid_id($id) {
//TODO: check: sane characters in id (i.e. alphanum + _) ?
}


function handle_get() {
//TODO

}


function handle_post() {
  $client_id = $_POST["client_id"];
  $client_pw = $_POST["client_pw"];

  if (is_valid_id($client_id) == false) {
    http_response_code(400); // 400 = Bad Request
    return;
  }

  // client handling
  if ($_POST["new_client"] == "true") {
    if (register_client($client_id, $client_pw) == false) {
      http_response_code(409); // 409 = Conflict
    }
    return;
  } else if (check_client($client_id, $client_pw) == false) {
    http_response_code(401); // 401 = Unauthorized
    return;
  }

  // game handling
  if ($_POST["new_game"] == "true") {
  //TODO initialize new game
    $game_id = $_POST["game_id"];
    if ($game_id == "") {
      $game_id = $client_id;
    }
    $game_pw = $_POST["game_pw"];

    $game_filename = $client_id . "__" . $game_id;
    if (file_exists($game_filename)) {
      http_response_code();
    }



  } else if ($_POST["join"] == "true") {
  //TODO handle join
  } else if ($_POST["move"] != "") {
  //TODO
  } else if ($_POST["give_up"] == "true") {
  //TODO
  } else if ($_POST["undo"] == "true") {
  //TODO
  } else if ($_POST["redo"] == "true") {
  //TODO
  }
}




$req_method = $_SERVER['REQUEST_METHOD'];

switch($req_method) {
 case "GET":
  handle_get();
  break;
 case "POST":
  handle_post();
  break;

 default:
  break;
};




/*

common variables:

 client_id
 game_id
 client_pw
 game_pw

*/




/*
GET vars:

  update
  game_list

*/










?>
