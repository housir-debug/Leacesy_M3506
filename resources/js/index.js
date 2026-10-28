class InstrumentApp {
    constructor() {
        this.ws = null;
        this.commands = [];
        this.commandButtons = [];
        this.loadCommands();
        this.initWebSocket();
        this.setupEventListeners();
    }

    async loadCommands() {
        try {
            const response = await fetch('/api/scpi_commands');
            if (!response.ok) {
                throw new Error(`Command list request failed with status ${response.status}`);
            }

            const data = await response.json();
            if (!Array.isArray(data.commands) || !data.commands.every((command) => typeof command === 'string')) {
                throw new Error('Command list response is invalid');
            }

            this.commands = data.commands;
            const list = document.getElementById('command-list');
            this.commandButtons = this.commands.map((command) => {
                const button = document.createElement('button');
                button.type = 'button';
                button.className = 'command-item';
                button.textContent = command;
                button.setAttribute('aria-label', `将命令填入输入框：${command}`);
                button.addEventListener('click', () => {
                    document.getElementById('command-input').value = command;
                    this.commandButtons.forEach((item) => {
                        item.classList.toggle('selected', item === button);
                    });
                    document.getElementById('command-input').focus();
                });
                list.appendChild(button);
                return button;
            });

            this.filterCommands();
        } catch (error) {
            console.error('Failed to load SCPI commands:', error);
            const status = document.getElementById('command-list-status');
            if (status) {
                status.textContent = '命令列表暂不可用，请连接设备服务。';
            }
        }
    }

    commandMatchScore(command, query) {
        const normalizedCommand = command.toLowerCase();
        const exactIndex = normalizedCommand.indexOf(query);
        if (exactIndex !== -1) {
            return exactIndex + (normalizedCommand.length - query.length) * 0.01;
        }

        let queryIndex = 0;
        let previousMatch = -1;
        let firstMatch = -1;
        let gapPenalty = 0;

        for (let index = 0; index < normalizedCommand.length && queryIndex < query.length; index += 1) {
            if (normalizedCommand[index] !== query[queryIndex]) continue;
            if (firstMatch === -1) firstMatch = index;
            if (previousMatch !== -1) gapPenalty += index - previousMatch - 1;
            previousMatch = index;
            queryIndex += 1;
        }

        if (queryIndex !== query.length) return Number.POSITIVE_INFINITY;
        return 20 + firstMatch + gapPenalty * 2 + (normalizedCommand.length - query.length) * 0.01;
    }

    filterCommands() {
        const search = document.getElementById('command-search');
        const status = document.getElementById('command-list-status');
        const list = document.getElementById('command-list');
        if (!search || !status || !list) return;

        const query = search.value.trim().toLowerCase().replace(/\s+/g, '');
        const ranked = this.commands
            .map((command, index) => ({
                command,
                index,
                score: query ? this.commandMatchScore(command, query) : index
            }))
            .filter((match) => Number.isFinite(match.score))
            .sort((left, right) => left.score - right.score || left.index - right.index);

        list.innerHTML = '';
        const buttonMap = new Map(this.commandButtons.map(b => [b.textContent, b]));
        ranked.forEach(({ command }) => {
            const button = buttonMap.get(command);
            if (button) list.appendChild(button);
        });

        if (this.commands.length === 0) {
            status.textContent = '暂无可用命令';
            status.hidden = false;
        } else if (ranked.length === 0) {
            status.textContent = '没有匹配的命令';
            status.hidden = false;
        } else {
            status.textContent = '设备支持命令如上所示';
            status.hidden = true;
        }
    }

    initWebSocket() {
        const wsUrl = `ws://${window.location.hostname}:8080`;
        this.ws = new WebSocket(wsUrl);

        this.ws.onopen = () => {
            console.log('Connected to Instrument WebSocket:', wsUrl);
            window.sharedComponents.updateServiceStatus(true);
        };

        this.ws.onmessage = (event) => {
            const data = JSON.parse(event.data);
            if (data.type === 'scpi_response') {
                this.addToOutput(`> ${data.result}`, 'response');
            }
        };

        this.ws.onclose = () => {
            console.warn('Instrument WebSocket disconnected');
            window.sharedComponents.updateServiceStatus(false);
        };

        this.ws.onerror = () => {
            window.sharedComponents.updateServiceStatus(false);
        };
    }

    setupEventListeners() {
        const input = document.getElementById('command-input');
        if (input) {
            input.addEventListener('keydown', (event) => {
                if (event.key === 'Enter') this.sendCommand();
            });
        }

        const sendButton = document.getElementById('send-button');
        if (sendButton) {
            sendButton.addEventListener('click', () => this.sendCommand());
        }

        const search = document.getElementById('command-search');
        if (search) {
            search.addEventListener('input', () => this.filterCommands());
        }
    }

    sendCommand() {
        const input = document.getElementById('command-input');
        const command = input.value.trim();
        if (!command) return;

        this.addToOutput(`> ${command}`, 'command');
        input.value = '';
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            this.ws.send(JSON.stringify({
                type: 'scpi_command',
                command
            }));
        } else {
            this.addToOutput('设备未连接，指令未发送。', 'error');
        }
    }

    addToOutput(text, type) {
        const output = document.getElementById('output');
        const line = document.createElement('div');
        line.className = `output-line ${type}`;
        line.textContent = text;
        output.appendChild(line);

        while (output.children.length > 36) {
            output.removeChild(output.firstChild);
        }

        output.scrollTop = output.scrollHeight;
    }

    destroy() {
        if (this.ws) this.ws.close();
    }
}

document.addEventListener('DOMContentLoaded', () => {
    window.app = new InstrumentApp();
});
