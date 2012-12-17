 var users = [];
var ants = [];
var unsynced_users = [];
var synced_users = [];
var unsynced_ants = [];
var synced_ants = [];


var selected_user;
var selected_ant;

var current_table_array = [];
var current_ant_table_array = [];

function load_users() {

  $.getJSON('http://api.olinexpo.com/users/', function (data, successflag) {

      if (!successflag) {
        console.log("Error: No user data was returned");
        return;
      }
    users = data;
    sortUsersByCreationDate();

    extract_user_sync_state();
    updateTableWithUsers(unsynced_users);
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
      synced_users.push(ant.user);
    }
  });
}
}

function extract_user_sync_state() {
  if (ants && users) {}
    $.each(users, function(index, user) {
      if (synced_users.indexOf(user.id.toString()) == -1) {
          unsynced_users.push(user);
      }
  });
 }

$('#users-search').on('keyup', function () {

  searchTableForCellsMatchingTermInSearchBar($('#users-search').val().toLowerCase(), users);
});


$('#ant-search').on('keyup', function () {
  searchAntTable($('#ant-search').val().toLowerCase());
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
  $('#users-search').val("");

  $('#selected-user-span').text("None").removeClass('selected-user-text');
  $('#ant-select-container').fadeOut();
  $('#selected-ant-container').fadeOut();
  $('#sync-button').fadeOut();
  updateTableWithUsers(unsynced_users);
}

function clearAntsTableAndSearch() {
  $('#ant-search').val("");

  $('#selected-ant-span').text("None").removeClass('selected-user-text');

  $('#sync-button').fadeOut();

  updateTableWithAnts(unsynced_ants);

}

function tableContainsUser(table_array, user_to_find) {
  var found = false;
  $.each(table_array, function(index, user) {
    if (user.id == user_to_find.id) { 
      found = true;
      return false;
    }
  });

  return found;
}

function searchAntTable(filter_term) {
  var filtered_ants = [];

    if (!filter_term) {
      updateTableWithAnts(unsynced_ants);
      return;
    } 

    $.each(current_ant_table_array, function(index, ant) {

      if (ant.id && ant.id.toLowerCase().indexOf(filter_term) != -1) {
          filtered_ants.push(ant);
      }
    });

    updateTableWithAnts(filtered_ants);
}

function antTableRowClicked() {
  var new_selection = $(this).data();

  var same = false;

  $('#ants-table tr.table-row-selected').each(function(index, row) { 
      $(row).removeClass('table-row-selected');
  });

  if (new_selection != selected_ant) {
    selected_ant = new_selection;

    $(this).addClass('table-row-selected');

    $('#selected-ant-span').text(selected_ant.id).addClass('selected-user-text');

    $('#sync-button').fadeIn();
  }
  else {

    selected_ant = null;

    $('#selected-ant-span').text("None").removeClass('selected-user-text');

    $('#sync-button').fadeOut();
  }
}

function tableRowClicked() {

  var new_selection = $(this).data();

  var same = false;

  $('.table-row-selected').each(function(index, row) { 
      $(row).removeClass('table-row-selected');
  });

  if (new_selection != selected_user) {

    $(this).addClass("table-row-selected");

    selected_user = new_selection;

    var names = selected_user.name.split(" ");
    var display_name = "";

    for (var i = 0; i < names.length && i < 2; i++) {
      if (i) {
        display_name += " ";
      }
      display_name = display_name + names[i];
    }



    $('#selected-user-span').text(display_name).addClass('selected-user-text');

    $('#ant-select-container').fadeIn();

    $('#selected-ant-container').fadeIn();

  } else {
    selected_user = null;

    $('#selected-user-span').text("None").removeClass('selected-user-text');

    $('#ant-select-container').fadeOut();

    $('#selected-ant-container').fadeOut();
  }
}

function updateTableWithUsers(users_to_table) {

  current_table_array = users_to_table;

  user_table = $('#users-table');

  
  user_table.empty();

  var header = $('<thead>');
  var header_first = $('<th>').text("First Name");
  var header_last = $('<th>').text("Last Name");
  var header_created = $('<th>').text("Created At");
  header.append(header_first, header_last, header_created);
  user_table.append(header);


  $.each(users_to_table, function(index, user) {
    if (user.name) {
      var row = $('<tr>').addClass('table-row').click(tableRowClicked);
      var firstname = $('<td>').text(user.name.split(" ")[0]);
      var lastname = $('<td>').text(user.name.split(" ")[1]);
      var createdAt = $('<td>').text(humanReadableDateFromUser(user));
      $(row).data(user);
      row.append(firstname, lastname, createdAt);
      user_table.append(row);
    }
  });
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

  return (month + "/" + day + "/" + year + "\t" + hours + ":" + minutes + " " + meridian) ;
}
function updateTableWithAnts(ants_to_table) {

  var ants_table = $('#ants-table').empty();

  current_ant_table_array = ants_to_table;

  $.each(ants_to_table, function(index, ant) {
    if (ant.id) {
      var row = $('<tr>').addClass('table-row').click(antTableRowClicked);
      var ant_id = $('<td>').text(ant.id);  
      $(row).data(ant);
      row.append(ant_id);
      ants_table.append(row);
    }
  });
}

function load_ants() {
  $.getJSON('http://api.olinexpo.com/ants/', function (data, successflag) {

  if (!successflag) {
    console.log("Error: No ant data was returned");
    return;
  }

  ants = data;

  extract_ant_sync_state();

  updateTableWithAnts(unsynced_ants);

  load_users();
  });
}

$('#sync-user').click(function() {

   $.ajax({
    type: 'PUT',
    url: 'http://api.olinexpo.com/ants/' + selected_ant.id ,
    data: {
      user:selected_user.id
    },
    success: function (ret, successflag) {
      if (successflag) {
        $('#sync-text').text("Sync succesful").css("color", "green");

      } else {
        $('#sync-text').text("Sync FAILED").css("color", "red");
      }

      $("#dialog").show();

      setTimeout(function() {
          window.location.reload();
        }, 1000);
    }
  })
});


$(load_ants);

