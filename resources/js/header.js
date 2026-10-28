/* ========== Shared Header Component ========== */
const headerHTML = `
<header class="app-header">
    <div class="device-overview">
        <a class="brand-mark" href="./index.html" aria-label="Instrument 首页">
            <img src="./icon/leacesyicon.png" alt="Leacesy" class="logo-img">
        </a>
        <div class="device-metrics" aria-label="设备信息">
            <div class="device-metric">
                <span>Device Model</span>
                <strong id="model">读取中</strong>
            </div>
            <div class="device-metric">
                <span>Serial Number</span>
                <strong id="serial">读取中</strong>
            </div>
            <div class="device-metric">
                <span>Software Version</span>
                <strong id="software">读取中</strong>
            </div>
            <div class="device-metric">
                <span>Hardware Version</span>
                <strong id="hardware">读取中</strong>
            </div>
        </div>
    </div>
    <div class="header-toolbar">
        <div class="header-service">
            <div class="brand-status is-pending" role="status" aria-live="polite">
                <span class="status-indicator-small" aria-hidden="true"></span>
                <span id="header-status-text">设备连接中</span>
            </div>
            <a class="about-link" href="https://www.leacesy.com" target="_blank" rel="noopener noreferrer">关于我们</a>
        </div>
        <nav aria-label="主导航">
            <a href="./index.html" id="nav-console">通道中控</a>
            <a href="./channels.html" id="nav-channels">远程监控</a>
            <a href="./import.html" id="nav-import">模型管理</a>
        </nav>
    </div>
</header>
`;

async function loadDeviceInfo() {
    try {
        const response = await fetch('/api/device/info');
        if (!response.ok) {
            throw new Error(`Device info request failed with status ${response.status}`);
        }

        const data = await response.json();
        document.getElementById('model').textContent = data.model || '暂无数据';
        document.getElementById('serial').textContent = data.serial || '暂无数据';
        document.getElementById('software').textContent = data.software || '暂无数据';
        document.getElementById('hardware').textContent = data.hardware || '暂无数据';
    } catch (error) {
        console.error('Failed to load device info:', error);
        ['model', 'serial', 'software', 'hardware'].forEach((id) => {
            document.getElementById(id).textContent = '不可用';
        });
    }
}

function updateServiceStatus(isOnline) {
    const status = document.querySelector('.brand-status');
    const indicator = document.querySelector('.status-indicator-small');
    const text = document.getElementById('header-status-text');
    if (!status || !indicator || !text) return;

    status.classList.toggle('is-online', isOnline);
    status.classList.toggle('is-offline', !isOnline);
    status.classList.remove('is-pending');
    indicator.classList.toggle('is-online', isOnline);
    indicator.classList.toggle('is-offline', !isOnline);
    text.textContent = isOnline ? '设备已连接' : '设备连接异常';
}

function loadHeader(containerId, page) {
    const container = document.getElementById(containerId);
    if (!container) return;

    container.innerHTML = headerHTML;
    const navLink = document.getElementById(`nav-${page}`);
    if (navLink) {
        navLink.classList.add('active');
    }
    loadDeviceInfo();
}

window.sharedComponents = {
    loadHeader,
    updateServiceStatus
};
