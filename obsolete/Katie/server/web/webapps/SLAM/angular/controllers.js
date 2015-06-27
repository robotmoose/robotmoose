var robotId;

function LogListControl($scope, $http) {
    var pageSize = 10;

    $scope.loadList = function(pageNumber, size) {
        if (size !== undefined) {
            pageSize = size;
        }
        console.log("LogListControl: load page " + pageNumber + ", robot " + robotId);
        $http({method: "POST", url: "showDataLogs", params: {pageSize: pageSize, pageNumber: pageNumber, robotId: robotId}}).success(function(data) {
            $scope.logList = data.data;

            $scope.totalItems = data.totalItems;
            $scope.itemsPerPage = pageSize;
            $scope.currentPage = pageNumber;
            $scope.maxSize = 5;
        });
    };

    $scope.showLogFile = function(path) {
        window.open(path, '', 'width=640,height=480');
    };
}

function ExperimentListControl($scope, $http) {
    var pageSize = 10;

    $scope.loadList = function(pageNumber, size) {
        if (size !== undefined) {
            pageSize = size;
        }
        console.log("ExperimentListControl: load page " + pageNumber + ", robot " + robotId);
        $http({method: "POST", url: "showExperiments", params: {pageSize: pageSize, pageNumber: pageNumber, robotId: robotId}}).success(function(data) {
            $scope.logList = data.data;

            $scope.totalItems = data.totalItems;
            $scope.itemsPerPage = pageSize;
            $scope.currentPage = pageNumber;
            $scope.maxSize = 5;
        });
    };

    $scope.goOpenNote = function(id) {
        openNote(id);
    };
}

function PhotoListControl($scope, $http) {
    var pageSize = 4;

    $scope.loadList = function(pageNumber) {
        console.log("PhotoListControl: load page " + pageNumber + ", robot " + robotId);
        var postParams;
        if ($scope.useTag) {
            postParams = {pageSize: pageSize, pageNumber: pageNumber, robotId: robotId, tag: $scope.currentTag};
        } else {
            postParams = {pageSize: pageSize, pageNumber: pageNumber, robotId: robotId};
        }
        $http({method: "POST", url: "showPhotos", params: postParams}).success(function(data) {
            $scope.photoList = data.data;

            $scope.totalItems = data.totalItems;
            $scope.itemsPerPage = pageSize;
            $scope.currentPage = pageNumber;
            $scope.maxSize = 5;
        });
    };

    $scope.showPhoto = function(path) {
        window.open("photo.html?photoId=" + path, "", "width=640,height=480");
    };

    $scope.goTargetPage = function() {
        if (robotId === undefined) {
            alert("Please choose a robot first!");
            return;
        }
        $scope.loadList($scope.targetPage);
    };

    $scope.currentTag = "Choose a tag";
    $scope.useTag = false;
    $scope.loadTags = function() {
        $http({method: "POST", url: "listAllTag"}).success(function(data) {
            $scope.allTags = data.tags;
            $scope.allTags.unshift("{Display All}");
        });
    };

    $scope.startChooseTag = function() {
        $scope.loadTags();
    };

    $scope.chooseTag = function(tag) {
        if (robotId === undefined) {
            alert("Please choose a robot first!");
            return;
        }
        $scope.currentTag = tag;
        if (tag === "{Display All}") {
            $scope.useTag = false;
        } else {
            $scope.useTag = true;
        }
        $scope.loadList(1);
    };
}

function OnePhotoListControl($scope, $http) {
    var pageSize = 1;
    var pageNumber = 1;

    $scope.loadList = function() {
        console.log("PhotoListControl: load page " + pageNumber + ", robot " + robotId);
        var postParams = {pageSize: pageSize, pageNumber: pageNumber, robotId: robotId};
        $http({method: "POST", url: "showPhotos", params: postParams}).success(function(data) {
            var total = data.totalItems;
            if (total >= pageNumber) {
                $scope.canShowPrevious = false;
            } else {
                $scope.canShowPrevious = true;
            }
            $scope.photo = data.data[0];
            if (pageNumber === 1) {
                $scope.$broadcast("timer-stop");
                $scope.startTime = $scope.photo.saveTimeValue;
                $scope.showTimer = true;
                $scope.imageTitle = "Lastest Image";
                try {
                    $scope.$apply();
                } catch (e) {
                }
                $scope.$broadcast("timer-start");
            } else {
                $scope.$broadcast("timer-stop");
                $scope.showTimer = false;
                $scope.imageTitle = "Image";
            }
        });
    };

    $scope.showPhoto = function(path) {
        window.open("photo.html?photoId=" + path, "", "width=640,height=480");
    };

    $scope.showLatest = function() {
        pageNumber = 1;
        $scope.loadList();
    };

    $scope.showPrevious = function() {
        pageNumber += 1;
        $scope.loadList();
    };

    $scope.cameraTilt = 90;
    $scope.tiltCamera = function() {
        if (!canSendCommand($scope)) {
            return;
        }
        ws.send(JSON.stringify(new ControlMessage(null, COMMAND_TYPES[14], {
            robotId: robotId,
            cameraTilt: $scope.cameraTilt
        })));
    };
}

function PhotoInfoControl($scope, $http, $location) {
    var photoId = $location.search().photoId;
    $scope.tagsDraw = [];

    $scope.loadPhoto = function(photoId) {
        $http({method: "POST", url: "getPhotoInfo", params: {photoId: photoId}}).success(function(data) {
            $scope.photo = data.photo;
            $scope.tags = data.tags;
        });
    };

    $scope.loadAreas = function() {
        $http({method: "POST", url: "listDrawTag", params: {photo: photoId}}).success(function(data) {
            for (var i = 0; i < data.tags.length; i++) {
                $scope.tagsDraw.push(data.tags[i].tag);
                document.getElementById("photoIFrame").contentWindow.newRect(
                        data.tags[i].x,
                        data.tags[i].y,
                        data.tags[i].width,
                        data.tags[i].height,
                        data.tags[i].tag);
            }
        });
    };

    $scope.newTag = function(tag) {
        $http({method: "POST", url: "addTag", params: {photoId: photoId, tag: tag}}).error(function() {
            alert("Cannot add the tag");
            $scope.tags.pop();
        });
    };

    $scope.removeTag = function(tag) {
        $http({method: "POST", url: "removeTag", params: {photoId: photoId, tag: tag}}).error(function() {
            alert("Cannot remove the tag");
        });
    };

    $scope.getPhotoId = function() {
        return photoId;
    };

    $scope.getPhotoUrl = function() {
        return $scope.photo.filePath;
    };


    $scope.addNewTag = function(tag) {
        $scope.tagsDraw.push(tag.tag);
        $http({method: "POST", url: "addDrawTag", params: {tagJson: tag}}).error(function() {
            alert("Cannot send tag to server");
        });
    };

    $scope.cancelAddDrawingTag = function(tag) {
        alert("Please do not type your drawing tag");
        $scope.tagsDraw.pop();
    };

    $scope.removeDrawingTag = function(tag) {
        document.getElementById("photoIFrame").contentWindow.removeTag(tag);
        $http({method: "POST", url: "removeDrawTag", params: {photo: photoId, tag: tag}}).error(function() {
            alert("Cannot remove the tag");
        });
    };

    $scope.loadPhoto(photoId);
}

function VideoListControl($scope, $http) {
    var pageSize = 10;

    var button = new Object();
    button.text = "Start Recording";
    $scope.button = button;

    $scope.loadList = function(pageNumber) {
        $http({method: "POST", url: "viewVideoList", params: {pageSize: pageSize, pageNumber: pageNumber}}).success(function(data) {
            $scope.videoList = data.data;

            $scope.totalItems = data.totalItems;
            $scope.itemsPerPage = pageSize;
            $scope.currentPage = pageNumber;
            $scope.maxSize = 5;
        });
    };

    $scope.loadList(1);

    $scope.startRecording = function() {
        if ($scope.button.text === "Start Recording") {
            var ip = $scope.cameraIP;
            $http({method: "POST", url: "record", params: {ip: ip, controll: "start"}}).success(function(data) {
                $scope.loadList();
                alert("Recording started");
                button.text = "Stop Recording";
            }).error(function(data, status, headers, config) {
                alert("Cannot start recording");
                console.log(data);
                console.log(status);
            });

        } else if ($scope.button.text === "Stop Recording") {
            $http({method: "POST", url: "record", params: {ip: "dummy", controll: "end"}}).success(function(data) {
                $scope.loadList();
            });
            button.text = "Start Recording";
        }
    };
}

function RobotInfoControl($scope) {
    $scope.robots = [{name: "Katie White : 0", robotId: 0},
        {name: "Katie Blue : 1", robotId: 1},
        {name: "Katie Yellow : 2", robotId: 2}];

    $scope.showRobot = function(robot) {
        robotId = robot.robotId;
        angular.element(document.getElementById("LogListControl")).scope().loadList(1, 5);
        angular.element(document.getElementById("PhotoListControl")).scope().loadList(1);
        angular.element(document.getElementById("ExperimentListControl")).scope().loadList(1, 5);
        angular.element(document.getElementById("OnePhotoListControl")).scope().loadList();
        ws.send(JSON.stringify(new ControlMessage(null, COMMAND_TYPES[0], {robotId: robotId})));
    };
}

function canSendCommand(scope) {
    if (robotId === undefined) {
        alert("Please choose a robot first!");
        return false;
    }
    if (scope.online !== "online") {
        scope.stateQuickTurn = 0;
        scope.stateE = 0;
        scope.stateT = 0;
        scope.stateExplore = 0;
        scope.stateRoomba = 0;
        scope.takeIsOn = false;
        alert("Robot not online");
        return false;
    }
    return true;
}

function RobotControl($scope, $modal) {
    $scope.online = "";
    $scope.stateQuickTurn = 0;
    $scope.stateE = 0;
    $scope.stateT = 0;
    $scope.stateExplore = 0;
    $scope.stateRoomba = 0;
    $scope.takeIsOn = false;
    $scope.showExecutionTimer = false;
    $scope.showPerceptionTimer = false;

    $scope.setOnline = function(online) {
        if (online) {
            $scope.online = "online";
        } else {
            $scope.online = "offline";
            $scope.stateQuickTurn = 0;
            $scope.stateE = 0;
            $scope.stateT = 0;
            $scope.stateExplore = 0;
            $scope.stateRoomba = 0;
            $scope.takeIsOn = false;
            $scope.showExecutionTimer = false;
            $scope.showPerceptionTimer = false;
            $scope.$broadcast("timer-stop");
        }
        $scope.$apply();
    };

    $scope.setPerception = function(p) {
        $scope.perception = p;
        $scope.perceptionTime = new Date().getTime();
        $scope.showPerceptionTimer = true;
        $scope.$apply();
        $scope.$broadcast("timer-start");
    };

    $scope.setCommandExecution = function(c) {
        if (c) {
            $scope.commandExecution = "I have executed a command";
        } else {
            $scope.commandExecution = "Failed to execute command";
        }
        $scope.commandExecutionTime = new Date().getTime();
        $scope.showExecutionTimer = true;
        $scope.$apply();
        $scope.$broadcast("timer-start");
    };

    $scope.setCurrentHeading = function(c) {
        $scope.currentHeadingString = c;
    };

    $scope.setRearDistance = function(r) {
        $scope.rearDistanceString = r;
    };

    $scope.refresh = function() {
        angular.element(document.getElementById("LogListControl")).scope().loadList(1, 5);
        angular.element(document.getElementById("PhotoListControl")).scope().loadList(1);
        angular.element(document.getElementById("ExperimentListControl")).scope().loadList(1, 5);
        angular.element(document.getElementById("OnePhotoListControl")).scope().showLatest();
    };

    $scope.resetKaite = function() {
        if (!canSendCommand($scope)) {
            return;
        }
        ws.send(JSON.stringify(new ControlMessage(null, COMMAND_TYPES[1], {robotId: robotId})));
    };


    $scope.setQuickTurn = function(a) {
        $scope.stateQuickTurn = a;
    };
    $scope.quickTurn = function() {
        if (!canSendCommand($scope)) {
            $scope.stateQuickTurn = 1;
            return;
        }
        ws.send(JSON.stringify(new ControlMessage(null, COMMAND_TYPES[2], {robotId: robotId})));
    };

    $scope.setExplore = function(a) {
        $scope.stateExplore = a;
    };
    $scope.doExplore = function() {
        if (!canSendCommand($scope)) {
            $scope.stateExplore = 1;
            return;
        }
        ws.send(JSON.stringify(new ControlMessage(null, COMMAND_TYPES[3], {robotId: robotId})));
    };

    $scope.fullTurn = function() {
        if (!canSendCommand($scope)) {
            return;
        }
        ws.send(JSON.stringify(new ControlMessage(null, COMMAND_TYPES[4], {robotId: robotId})));
    };

    $scope.setModel = function(e, t) {
        $scope.stateE = e;
        $scope.stateT = t;
    };
    $scope.modelE = function() {
        if (!canSendCommand($scope)) {
            $scope.stateE = 1;
            return;
        }
        if ($scope.stateE === 0) {
            $scope.stateT = 0;
        }
        ws.send(JSON.stringify(new ControlMessage(null, COMMAND_TYPES[5], {robotId: robotId})));
    };
    $scope.modelT = function() {
        if (!canSendCommand($scope)) {
            $scope.stateT = 1;
            return;
        }
        if ($scope.stateT === 0) {
            $scope.stateE = 0;
        }
        ws.send(JSON.stringify(new ControlMessage(null, COMMAND_TYPES[6], {robotId: robotId})));
    };

    $scope.setRoomba = function(a) {
        $scope.stateRoomba = a;
    };
    $scope.doRoomba = function() {
        if (!canSendCommand($scope)) {
            $scope.stateRoomba = 1;
            return;
        }
        ws.send(JSON.stringify(new ControlMessage(null, COMMAND_TYPES[7], {robotId: robotId})));
    };

    $scope.rotation = 0;
    $scope.setRotation = function(degree) {
        $scope.rotation = degree;
    };

    $scope.rotationDegrees = [{value: 0, click: $scope.setRotation},
        {value: 90, click: $scope.setRotation},
        {value: 180, click: $scope.setRotation},
        {value: 270, click: $scope.setRotation}];

    $scope.setInterval = function(interval) {
        $scope.interval = interval;
    };
    $scope.formNotValid = false;
    $scope.checkFormValid = function() {
        if ($scope.interval <= 0) {
            $scope.formNotValid = true;
        } else {
            $scope.formNotValid = false;
        }
    };
    $scope.$watch("interval", function() {
        $scope.checkFormValid();
    });

    $scope.setAutoTake = function(topState, buttonState) {
        $scope.takeIsOn = topState;
        $scope.stateAutoTake = buttonState;
    };
    $scope.autoTakePhoto = function() {
        if (!canSendCommand($scope)) {
            $scope.stateAutoTake = 1;
            return;
        }
        ws.send(JSON.stringify(new ControlMessage(null, COMMAND_TYPES[8], {interval: $scope.interval, rotation: $scope.rotation, robotId: robotId})));
        $scope.takeIsOn = !$scope.takeIsOn;
    };

    $scope.takeOnePhoto = function() {
        if (!canSendCommand($scope)) {
            return;
        }
        ws.send(JSON.stringify(new ControlMessage(null, COMMAND_TYPES[9], {robotId: robotId, rotation: $scope.rotation})));
    };

    $scope.driveDegrees = 0;
    $scope.driveDistance = 0;
    $scope.driveVelocity = 200;
    $scope.driveRadius = 0;
    $scope.showDegree = function(value) {
        return value.toString() + "°";
    };
    $scope.showMM = function(value) {
        return value.toString() + " mm";
    };
    $scope.showPercentage = function(value) {
        return value.toString() + "%";
    };
    $scope.showCM = function(value) {
        return value.toString() + " cm";
    };

    $scope.checkForm2Valid = function() {
        if ($scope.driveDistance < -999 || $scope.driveDistance > 999 || $scope.driveVelocity <= 0 || $scope.driveVelocity > 999) {
            $scope.form2NotValid = true;
        } else {
            $scope.form2NotValid = false;
        }
    };
    $scope.$watch("driveDistance", function() {
        $scope.checkForm2Valid();
    });
    $scope.$watch("driveVelocity", function() {
        $scope.checkForm2Valid();
    });

    $scope.driveOneStep = function() {
        if (!canSendCommand($scope)) {
            return;
        }
        ws.send(JSON.stringify(new ControlMessage(null, COMMAND_TYPES[10],
                {driveDistance: $scope.driveDistance,
                    driveVelocity: $scope.driveVelocity,
                    driveDegrees: $scope.driveDegrees,
                    driveRadius: $scope.driveRadius,
                    robotId: robotId
                })));
        $scope.driveDistance = 0;
        $scope.driveVelocity = 200;
        $scope.driveDegrees = 0;
        $scope.driveRadius = 0;
    };

    $scope.haltDrive = function() {
        if (!canSendCommand($scope)) {
            return;
        }
        ws.send(JSON.stringify(new ControlMessage(null, COMMAND_TYPES[11], {robotId: robotId})));
    };

    $scope.checkWidth = 1;
    $scope.checkSpeed = 50;
    $scope.checkRepeat = 1;
    $scope.randomness = 0;
    $scope.flightDistance = 1;
    $scope.flightSpeed = 150;

    $scope.checkShooValid = function() {
        if ($scope.checkSpeed < 150 || $scope.checkSpeed > 400) {
            $scope.shooNotValid = true;
        } else {
            $scope.shooNotValid = false;
        }
    };
    $scope.checkGreetValid = function() {
        if ($scope.checkWidth < 1 || $scope.checkWidth > 45 || $scope.checkSpeed < 50 || $scope.checkSpeed > 300) {
            $scope.greetNotValid = true;
        } else {
            $scope.greetNotValid = false;
        }
    };
    $scope.$watch("checkWidth", function() {
        $scope.checkGreetValid();
    });
    $scope.$watch("checkSpeed", function() {
        $scope.checkShooValid();
        $scope.checkGreetValid();
    });

    $scope.shoo = function() {
        if (!canSendCommand($scope)) {
            return;
        }
        ws.send(JSON.stringify(new ControlMessage(null, COMMAND_TYPES[12], {
            robotId: robotId,
            checkWidth: $scope.checkWidth,
            checkSpeed: $scope.checkSpeed,
            flightDistance: $scope.flightDistance,
            flightSpeed: $scope.flightSpeed,
            randomness: $scope.randomness
        })));
    };

    $scope.greet = function() {
        if (!canSendCommand($scope)) {
            return;
        }
        ws.send(JSON.stringify(new ControlMessage(null, COMMAND_TYPES[13], {
            robotId: robotId,
            checkWidth: $scope.checkWidth,
            checkSpeed: $scope.checkSpeed,
            checkRepeat: $scope.checkRepeat,
            randomness: $scope.randomness
        })));
    };

    $scope.openModal = function(title, mode) {
        if (!canSendCommand($scope)) {
            return;
        }

        var modalInstance = $modal.open({
            templateUrl: "modal_content",
            controller: ModalInstanceControl,
            resolve: {
                mode: function() {
                    return mode;
                },
                title: function() {
                    return title;
                }
            }
        });

        modalInstance.result.then(function(r) {
            $scope.driveVelocity = r[0];
            $scope.driveDistance = r[1];
            $scope.driveRadius = r[2];
            $scope.driveDegrees = r[3];
            $scope.driveOneStep();
        });
    };

    $scope.beep = function() {
        if (!canSendCommand($scope)) {
            return;
        }
        ws.send(JSON.stringify(new ControlMessage(null, COMMAND_TYPES[15], {robotId: robotId})));
    };

    $scope.scan = function() {
        if (!canSendCommand($scope)) {
            return;
        }
        ws.send(JSON.stringify(new ControlMessage(null, COMMAND_TYPES[16], {robotId: robotId})));
    };
}

function ModalInstanceControl($scope, $modalInstance, mode, title) {
    $scope.title = title;

    switch (mode) {
        case "lf":
            $scope.hideModalDegree = true;
            break;
        case "f":
            $scope.hideModalRadius = true;
            $scope.hideModalDegree = true;
            break;
        case "rf":
            $scope.hideModalDegree = true;
            break;
        case "rl":
            $scope.hideModalDistance = true;
            $scope.hideModalRadius = true;
            break;
        case "rr":
            $scope.hideModalDistance = true;
            $scope.hideModalRadius = true;
            break;
        case "bl":
            $scope.hideModalDegree = true;
            break;
        case "b":
            $scope.hideModalRadius = true;
            $scope.hideModalDegree = true;
            break;
        case "br":
            $scope.hideModalDegree = true;
            break;
    }

    $scope.ok = function() {
        var in_velocity = $("#modal_drive_velocity");
        var in_distance = $("#modal_drive_distance");
        var in_radius = $("#modal_drive_radius");
        var in_degree = $("#modal_drive_degree");

        var velocity = 0;
        var distance = 0;
        var radius = 0;
        var degree = 0;

        switch (mode) {
            case "lf":
                velocity = in_velocity.val();
                distance = in_distance.val();
                radius = in_radius.val();
                break;
            case "f":
                velocity = in_velocity.val();
                distance = in_distance.val();
                if (distance <= 0 || velocity <= 0) {
                    alert("Please give postive values");
                    return;
                }
                break;
            case "rf":
                velocity = in_velocity.val();
                distance = in_distance.val();
                radius = in_radius.val();
                radius = -radius; // mrf 10/15
                break;
            case "rl":
                velocity = in_velocity.val();
                degree = in_degree.val();
                degree = -degree; // changed 10/15 mrf
                break;
            case "rr":
                velocity = in_velocity.val();
                degree = in_degree.val();
                break;
            case "bl":
                velocity = in_velocity.val();
                distance = in_distance.val();
                radius = in_radius.val();
                distance = -distance; // mrf 10/15
                break;
            case "b":
                velocity = in_velocity.val();
                distance = in_distance.val();
                if (distance <= 0 || velocity <= 0) {
                    alert("Please give postive values");
                    return;
                }
                distance = -distance;
                break;
            case "br":
                velocity = in_velocity.val();
                distance = in_distance.val();
                radius = in_radius.val();
                radius = -radius; /// mrf
                distance = -distance; ///mrf
                break;
        }

        $modalInstance.close([velocity, distance, radius, degree]);
    };

    $scope.cancel = function() {
        $modalInstance.dismiss("cancel");
    };
}

function DataGraphControl($scope) {
    $scope.open = function($event) {
        $event.preventDefault();
        $event.stopPropagation();

        $scope.opened = true;
    };

    $scope.dateOptions = {
        "year-format": "'yy'",
        "starting-day": 1
    };

    $scope.format = "shortDate";

    $scope.showGraph = function() {
        if (robotId === undefined) {
            alert("Please choose a robot first!");
            return;
        }
        if (!$scope.dt) {
            alert("Please choose a date first!");
            return;
        }
        var date = $scope.dt;
        var time = $scope.mytime;
        var startTime = new Date(date.getFullYear(), date.getMonth(), date.getDate(),
                time.getHours(), time.getMinutes(), 0, 0);

        makeGraph(robotId, startTime.getTime());
    };


    $scope.mytime = new Date();
    $scope.hstep = 1;
    $scope.mstep = 5;

    $scope.ismeridian = true;

    $scope.changed = function() {
        console.log("Time changed to: " + $scope.mytime);
    };
}

function RobotConnectionsControl($scope, $http) {
    $scope.loadList = function() {
        $http({method: "POST", url: "showRobots"}).success(function(data) {
            $scope.robotList = data.robots;
        });
    };

    $scope.loadList();
}



//----- Utility Functions -----

function openNote(id) {
    $.ajax({
        url: "getNote",
        type: "post",
        data: {experimentId: id},
        success: function(data) {
            bootbox.dialog({
                message: "<textarea class='bootbox-input' rows='8' id='bootbox_text'>" + data + "</textarea>",
                title: "Note for Experiment-" + id,
                buttons: {
                    danger: {
                        label: "Back",
                        className: "btn-danger"
                    },
                    success: {
                        label: "Save",
                        className: "btn-primary",
                        callback: function() {
                            var note = $("#bootbox_text").val();

                            $.ajax({
                                url: "saveNote",
                                type: "post",
                                data: {experimentId: id, note: note}
                            });
                        }
                    }
                }
            });
        }
    });
}