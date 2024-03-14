const Eventable = {
    emit(event, ...args) {
        ((this.handlers || [])[event] || []).forEach(cb => cb(this, ...args));
        return this;
    },
    on(event, cb) {
        if (!this.handlers)
            this.handlers = [];

        (this.handlers[event] = this.handlers[event] || []).push(cb);
        return this;
    },
};

class RotatorNotifier {
    constructor() {
        this.popup = document.getElementById('popup');
        this.btnsPopup = document.querySelectorAll(".btn-popup");

        for (const btn of this.btnsPopup) {
            btn.addEventListener("click", event => {
                this.popupOpen(event.currentTarget.dataset.name);
            });
        }

        this.popup.addEventListener("click", event => {
            if (!event.target.closest('.popup-contents')) {
                this.popupClose();
            }
        });
    }

    popupOpen(name) {
        const inner = this.popup.querySelector(`.popup-inner[data-handle="${name}"]`);
        this.popup.querySelectorAll('.popup-inner').forEach(el => el.classList[el === inner ? 'add' : 'remove']('shown'));
        if (inner)
            this.popup.classList.add("shown");
    }

    popupClose() {
        this.popup.classList.remove("shown");
    }
}

class RotatorSocket {
    constructor(notifier) {
        Object.assign(this, Eventable);

        this.notifier = notifier;
        this.socket = null;
        this.messageTimeout = null;

        //this.gateway = `ws://rotator.local/ws`;
        this.gateway = `ws://${window.location.hostname}/ws`;
    }

    setupWebSocket() {
        this.resetMessageTimeout();
        this.socket = new WebSocket(this.gateway);

        this.socket.onopen = () => {
            console.log('WebSocket connection opened.');
            this.notifier.popupClose();
            this.resetMessageTimeout();
        };

        this.socket.onmessage = (event) => {
            //console.log("Got new data: " + event.data);
            const data = JSON.parse(event.data);
            this.emit('message', data);
            this.resetMessageTimeout();
        };
    }

    resetMessageTimeout() {
        clearTimeout(this.messageTimeout);
        this.messageTimeout = setTimeout(() => {
            console.log('Connection timeout, closing and reconnecting...');
            this.notifier.popupOpen("lostconnection");

            this.socket.close();
            this.setupWebSocket();
        }, 10000);
    }

    send(data) {
        this.socket.send(JSON.stringify(data));
    }

    isConnected() {
        return this.socket && this.socket.readyState === WebSocket.OPEN;
    }
}
