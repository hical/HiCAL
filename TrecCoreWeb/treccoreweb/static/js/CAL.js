/* Mousetraps keyboard shortcuts */
Mousetrap.bind(['s', 'h', 'k'], function(e, key) {
    var current_doc_id = $('#cal-document').data("doc-id");
    if(key == 's') {
        send_judgment(current_doc_id, true, false, false, false, true);
    }
    else if(key == 'h') {
        send_judgment(current_doc_id, false, true, false, false, true);
    }
    else if(key == 'k') {
        send_judgment(current_doc_id, false, false, true, false, true);
    }

    //if(queue.getLength() == 0){
    //    console.log("Getting the next patch of documents to judge");
    //    update_documents_to_judge_list();
    //}
});

Mousetrap.bind(['ctrl+f', 'command+f'], function(e) {
    e.preventDefault();
    post_ctrlf();
    $( "#search_content" ).focus();
    return false;
});

var search_content_form = document.getElementById('search_content');
var search_content_form_mousetrap = new Mousetrap(search_content_form);
search_content_form_mousetrap.bind(['ctrl+f', 'command+f'], function(e) {
    $( "#search_content" ).focus();
    post_ctrlf();
    return false;
});


function document_isEmpty(){
    var current_doc_id = $('#cal-document').data("doc-id");
    if(current_doc_id == '' || current_doc_id == 'None'){
        return true;
    }
    return false;
}


function updateDocument(id, title, content){
    console.log("Updating document view for document id: " + id);
    $('#cal-document').attr("data-doc-id", id).data("doc-id", id);
    $("#document_title").html(title);
    $("#document_content").html(content);
}


function updateTimer(){
    $("#nav_timer_header").hide().fadeIn();

    timer.stop();
    timer.start({precision: 'secondTenths', callback: function (values) {
        $('#nav_timer_header').html(values.toString(['hours', 'minutes', 'seconds', 'secondTenths']));
    }});
}

function  updateCounter(key){
    if(key == "s"){
        $("#nav_stats_rel_header").hide().fadeIn().html(++relCounter);
    }else if(key == "h"){
        $("#nav_stats_nonrel_header").hide().fadeIn().html(++nonRelCounter);
    }else if (key == "k"){
        $("#nav_stats_ontopic_header").hide().fadeIn().html(++onTopicCounter);
    }
}


/* SEARCH BAR and highlighter */

$('#searchContentForm').submit(function (e) {
    e.preventDefault();
    $("#searchNext").click();
    return false;
});



$(function() {

  // the input field
  var $input = $("#search_content"),
    // clear button
    $clearBtn = $("button[data-search='clear']"),
    // prev button
    $prevBtn = $("button[data-search='prev']"),
    // next button
    $nextBtn = $("button[data-search='next']"),
    // the context where to search
    $content = $(".document-cal"),
    // jQuery object to save <mark> elements
    $results = "",
    // the class that will be appended to the current
    // focused element
    currentClass = "current",
    // top offset for the jump (the search bar)
    offsetTop = 50,
    // the current index of the focused element
    currentIndex = 0;

  /**
   * Jumps to the element matching the currentIndex
   */
  function jumpTo() {
    if ($results.length) {
      var position,
        $current = $results.eq(currentIndex);
      $results.removeClass(currentClass);
      if ($current.length) {
        $current.addClass(currentClass);
        position = $current.offset().top - offsetTop;
        window.scrollTo(0, position);
      }
    }
  }

  /**
   * Searches for the entered keyword in the
   * specified context on input
   */
  $input.on("input", function() {
  	var searchVal = this.value;
    $content.unmark({
      done: function() {
        $content.mark(searchVal, {
          separateWordSearch: true,
          done: function() {
            $results = $content.find("mark");
            currentIndex = 0;
            jumpTo();
          }
        });
      }
    });
  });

  /**
   * Clears the search
   */
  $clearBtn.on("click", function() {
    $content.unmark();
    $input.val("").focus();
  });

  /**
   * Next and previous search jump to
   */
  $nextBtn.add($prevBtn).on("click", function() {
    if ($results.length) {
      currentIndex += $(this).is($prevBtn) ? -1 : 1;
      if (currentIndex < 0) {
        currentIndex = $results.length - 1;
      }
      if (currentIndex > $results.length - 1) {
        currentIndex = 0;
      }
      jumpTo();
    }
  });
});
