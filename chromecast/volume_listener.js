var VolumeListener = function() {
    this._knownVolume = null;
};

VolumeListener.prototype.start = function() {
    var self = this;

    var url = CONTROL_URL + "/volumeInfo";
    if (this._knownVolume) {
        url = url + "?v=" + this._knownVolume;
    }

    var xhr = new XMLHttpRequest();
    xhr.open("GET", url, true);
    xhr.onload = function(ev) {
        if (this.status != 200 && this.status != 204) {
            console.log('Error while getting the volume info.');
            return;
        }

        if (this.status == 200) {
            // We really got new data.
            volumeInfo = JSON.parse(this.responseText);

            var volume = volumeInfo['volume_percent'];
            setReceiverVolume(volume/100, volumeInfo['muted']);
            self._knownVolume = volume;
        }

        self.start();
    };
    xhr.onerror = function(ev) {
        console.log('Could not get the volume info!');
    };
    xhr.send();
};

