var socket = io();
var container = document.getElementsByClassName('container')[0];
var modal = document.getElementById('modal-overlay');
var timer = document.getElementsByClassName('counter')[0];
var counterTags = document.getElementsByClassName('counter-tag');
counterTags.tagIndex = 0;
var img = document.getElementsByTagName('img')[0];
var IMGPATH = 'img/';

// DOM events
container.addEventListener('click', function(e){
	if(event.srcElement.type === "button") {
		socket.emit('take photo', { type : e.srcElement.value});
	}
	e.stopPropagation();
}, false);

// socket events
socket.on('transmit photo', function(filename){
	// console.log(filename);
	//hide the modal
	modal.classList.add('hide');
	//reset the modal state
	timer.innerHTML = '';
	for(var i = 0; i < counterTags.length; i++){
		counterTags[i].classList.add('hide');
	}
	img.setAttribute('src', IMGPATH + filename);
	counterTags.tagIndex = 0;
});


socket.on('countdown timer', function(filename){
	var time = filename;
	//no-op if class is already gone
	modal.classList.remove('hide');
	timer.innerHTML = time;

	if(parseInt(time, 10) === 1){
		setTimeout(function(){
			counterTags[counterTags.tagIndex].classList.remove('hide');
			counterTags.tagIndex++;
		}, 1000);
	}
	// console.log('timer: ' + time);
});
