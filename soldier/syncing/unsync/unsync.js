var users = [];
var ants = [];
var unsynced_users = [];
var synced_user_ids= [];
var synced_users = [];
var unsynced_ants = [];
var synced_ants = [];


var selected_user;
var selected_ant;

var current_table_array = [];


function load_ants() {
  $.getJSON('http://api.olinexpo.com/ants/', function (data, successflag) {

  if (!successflag) {
    console.log("Error: No ant data was returned");
    return;
  }

  ants = data;

  extract_ant_sync_state();

  load_users();
  });
}

function load_users() {

  $.getJSON('http://api.olinexpo.com/users/', function (data, successflag) {
    users = data;

    sortUsersByCreationDate();

    var user_table = $('#synced-users-table');

      if (!successflag) {
        console.log("Error: No user data was returned");
        return;
      }

      extract_user_sync_state();

      updateTableWithUsers(synced_users);
  });
}

function sortUsersByCreationDate() {
  users.sort(function(a, b) {
    return Date.parse(b.created_at) - Date.parse(a.created_at);
  });
}
function extract_ant_sync_state() {
if (ants) {
  $.each(ants, function(index, ant) {
    if (!ant.user) {
      unsynced_ants.push(ant);
    }  
    else {
      synced_ants.push(ant);
      synced_user_ids.push(ant.user);
    }
  });
}
}

function extract_user_sync_state() {
  if (ants && users) {}
    $.each(users, function(index, user) {
      if (synced_user_ids.indexOf(user.id.toString()) == -1) {
          unsynced_users.push(user);
      } else {
          synced_users.push(user);
      }
  });
 }

$('#synced-users-search').on('keyup', function () {

  searchTableForCellsMatchingTermInSearchBar($('#synced-users-search').val().toLowerCase(), users);
});


function searchTableForCellsMatchingTermInSearchBar(filter_term, user_list) {
    var filtered_users = [];

    if (!filter_term) {
      updateTableWithUsers(user_list);
      return;
    } 

    $.each(user_list, function(index, user) {

      if (user.name && user.name.toLowerCase().indexOf(filter_term) != -1) {
          filtered_users.push(user);
      }
    });

    updateTableWithUsers(filtered_users);
}

function clearUsersTableAndSearch(table_array, new_table_array) {
  $('#synced-users-search').val("");

  resetUserSelection();

  updateTableWithUsers(synced_users);
}

function tableRowClicked() {

  var new_selection = $(this).data();

  var same = false;

  $('.table-row-selected').each(function(index, row) { 
      $(row).removeClass('table-row-selected');
  });

  if (new_selection != selected_user) {

    makeRowSelection($(this));

  } else {
    resetUserSelection();
  }
}

function makeRowSelection(row) {

  row.addClass("table-row-selected");

  selected_user = row.data();

  selected_ant = antForUser(selected_user);

  var names = selected_user.name.split(" ");
  var display_name = "";

  for (var i = 0; i < names.length && i < 2; i++) {
    if (i) {
      display_name += " ";
    }
    display_name = display_name + names[i];
  }

  $('#selected-user-span').text(display_name).addClass('selected-user-text');

  $('#selected-ant-span').text(selected_ant.id).addClass('selected-user-text') ;

  $('#with-text').fadeIn();

  $('#selected-ant-container').fadeIn();

  $('#unsync-button').fadeIn();
}


function resetUserSelection() {

  selected_user = selected_ant = null;

  $('#selected-user-span').text("None").removeClass('selected-user-text');

  $('#unsync-button').hide();

  $('#with-text').hide();
}


function updateTableWithUsers(users_to_table) {

  current_table_array = users_to_table;

  user_table = $('#synced-users-table');
  
  user_table.empty();

  var header = $('<thead>');
  var header_first = $('<th>').text("First Name");
  var header_last = $('<th>').text("Last Name");
  var ant_header = $('<th>').text("Synced Ant ID");
  header.append(header_first, header_last, ant_header);
  user_table.append(header);


  $.each(users_to_table, function(index, user) {
    if (user.name) {
      var row = $('<tr>').addClass('table-row').click(tableRowClicked);
      var firstname = $('<td>').text(user.name.split(" ")[0]);
      var lastname = $('<td>').text(user.name.split(" ")[1]);
      var createdAt = $('<td>').text(antForUser(user).id);
      $(row).data(user);
      row.append(firstname, lastname, createdAt);
      user_table.append(row);
    }
  });
}

function antForUser(user) {
  var ret;

  $.each(synced_ants, function(index, ant) {
    if (ant.user == user.id) {
      ret = ant; 
      return false
    }
  });

  return ret;

}
function humanReadableDateFromUser(user) {
  var date = new Date(Date.parse(user.created_at));

  var d  = date.getDate();
  var day = (d < 10) ? '0' + d : d;
  var m = date.getMonth() + 1;
  var month = (m < 10) ? '0' + m : m;
  var yy = date.getYear();
  var year = (yy < 1000) ? yy + 1900 : yy;

  var hours = date.getHours();
  var minutes = date.getMinutes();
  var meridian; 

  if (hours >= 12) {
    meridian = "PM";
    if (hours > 12) hours = hours - 12;
  } else {
    if (hours == 0) {
      hours = 12;
    } else {
      '0' + hours;
    }
    meridian = "AM";
  }

  if (minutes < 10) {
    minutes = '0' + minutes;
  }

  return (month + "/" + day + "/" + year + "\t" + hours + ":" + minutes + " " + meridian);
}

$('#unsync-user').click(function() {

   $.ajax({
    type: 'PUT',
    url: 'http://api.olinexpo.com/ants/' + selected_ant.id ,
    data: {
    },
    success: function (ret, successflag) {
      if (successflag) {
        $('#unsync-text').text("Unync succesful").css("color", "green");

      } else {
        $('#unsync-text').text("Unync FAILED").css("color", "red");
      }

      $("#dialog").show();

      setTimeout(function() {
          window.location.reload();
        }, 1000);
    }
  })
});


$(load_ants);

