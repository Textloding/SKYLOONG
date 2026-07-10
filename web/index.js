"use strict";

const app = document.getElementById("app");
const toasts = document.getElementById("toasts");
const modals = document.getElementById("modals");

const SCREEN_W = 320;
const SCREEN_H = 240;
const VIDEO_TARGET_W = SCREEN_W;
const VIDEO_TARGET_H = SCREEN_H;
const VIDEO_TARGET_FPS = 24;
const VIDEO_BITRATE = "320k";
const VIDEO_MAXRATE = "420k";
const VIDEO_AUDIO_BITRATE = "32k";
const VIDEO_AUDIO_RATE = "44100";
const VIDEO_AUDIO_CHANNELS = "1";
const spaceBreathLabels = ["慢速", "标准", "快速"];
const $ = (sel, root = document) => root.querySelector(sel);
const $$ = (sel, root = document) => [...root.querySelectorAll(sel)];
const clamp = (n, min, max) => Math.max(min, Math.min(max, n));
const esc = (value = "") => String(value).replace(/[&<>"']/g, c => ({
  "&": "&amp;",
  "<": "&lt;",
  ">": "&gt;",
  '"': "&quot;",
  "'": "&#39;",
}[c]));

const icon = path =>
  `<svg viewBox="0 0 24 24" aria-hidden="true" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">${path}</svg>`;

const I = {
  logo: icon('<rect x="3" y="5" width="18" height="14" rx="3"/><path d="M7 10h5M7 14h8"/><circle cx="17" cy="10" r="1.4" fill="currentColor" stroke="none"/>'),
  overview: icon('<rect x="3" y="3" width="7" height="8" rx="2"/><rect x="14" y="3" width="7" height="5" rx="2"/><rect x="14" y="12" width="7" height="9" rx="2"/><rect x="3" y="15" width="7" height="6" rx="2"/>'),
  media: icon('<rect x="3" y="5" width="18" height="14" rx="3"/><path d="m10 9 5 3-5 3V9z" fill="currentColor" stroke="none"/>'),
  display: icon('<rect x="3" y="4" width="18" height="14" rx="3"/><path d="M8 21h8M12 18v3"/>'),
  network: icon('<path d="M2.5 8.5a15 15 0 0 1 19 0"/><path d="M6 12a9.5 9.5 0 0 1 12 0"/><path d="M9.3 15.5a4.2 4.2 0 0 1 5.4 0"/><circle cx="12" cy="19" r="1.4" fill="currentColor" stroke="none"/>'),
  system: icon('<circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.7 1.7 0 0 0 .3 1.9l.1.1a2 2 0 1 1-2.8 2.8l-.1-.1a1.7 1.7 0 0 0-1.9-.3 1.7 1.7 0 0 0-1 1.6V21a2 2 0 1 1-4 0v-.1a1.7 1.7 0 0 0-1-1.6 1.7 1.7 0 0 0-1.9.3l-.1.1A2 2 0 1 1 4.2 17l.1-.1a1.7 1.7 0 0 0 .3-1.9 1.7 1.7 0 0 0-1.6-1H3a2 2 0 1 1 0-4h.1a1.7 1.7 0 0 0 1.6-1 1.7 1.7 0 0 0-.3-1.9L4.3 7A2 2 0 1 1 7.1 4.2l.1.1a1.7 1.7 0 0 0 1.9.3h.1a1.7 1.7 0 0 0 1-1.6V3a2 2 0 1 1 4 0v.1a1.7 1.7 0 0 0 1 1.6 1.7 1.7 0 0 0 1.9-.3l.1-.1A2 2 0 1 1 19.8 7l-.1.1a1.7 1.7 0 0 0-.3 1.9v.1a1.7 1.7 0 0 0 1.6 1h.1a2 2 0 1 1 0 4h-.1a1.7 1.7 0 0 0-1.6 1z"/>'),
  image: icon('<rect x="3" y="5" width="18" height="14" rx="3"/><circle cx="9" cy="11" r="1.7"/><path d="m5.5 18 4.8-4.8a1.5 1.5 0 0 1 2.1 0L18 18"/><path d="m14.5 15.5 1.2-1.2a1.5 1.5 0 0 1 2.1 0l2.2 2.2"/>'),
  upload: icon('<path d="M12 16V4m0 0L7.5 8.5M12 4l4.5 4.5"/><path d="M4 20h16"/>'),
  trash: icon('<path d="M4 7h16M10 11v6M14 11v6M9 7l.6-3h4.8L15 7M6 7l1 13h10l1-13"/>'),
  check: icon('<path d="m4.5 12.5 5 5L19.5 7"/>'),
  clock: icon('<circle cx="12" cy="12" r="9"/><path d="M12 7v5l3 2"/>'),
  power: icon('<path d="M12 3v9"/><path d="M6.4 6.8a8 8 0 1 0 11.2 0"/>'),
  refresh: icon('<path d="M20 11a8 8 0 1 0-2.3 6.3"/><path d="M20 4v7h-7"/>'),
  lock: icon('<rect x="5" y="11" width="14" height="9" rx="2"/><path d="M8 11V8a4 4 0 0 1 8 0v3"/>'),
  eye: icon('<path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7-10-7-10-7z"/><circle cx="12" cy="12" r="3"/>'),
  eyeOff: icon('<path d="M3 3l18 18M10.7 5.2A9 9 0 0 1 12 5c6.5 0 10 7 10 7a16 16 0 0 1-3 3.9M6.6 6.6A16 16 0 0 0 2 12s3.5 7 10 7a9.7 9.7 0 0 0 4-.9"/><path d="M9.9 9.9a3 3 0 0 0 4.2 4.2"/>'),
  play: icon('<path d="M7 4.8v14.4a.8.8 0 0 0 1.2.7l12-7.2a.8.8 0 0 0 0-1.4l-12-7.2A.8.8 0 0 0 7 4.8z"/>'),
  stop: icon('<rect x="6" y="6" width="12" height="12" rx="2"/>'),
  music: icon('<path d="M9 18V6l10-2v11"/><circle cx="6.5" cy="18" r="2.5"/><circle cx="16.5" cy="15" r="2.5"/>'),
  location: icon('<path d="M20 10c0 5-8 11-8 11S4 15 4 10a8 8 0 1 1 16 0z"/><circle cx="12" cy="10" r="2.5"/>'),
  speaker: icon('<path d="M11 5 6 9H3v6h3l5 4V5z"/><path d="M16 8.5a5 5 0 0 1 0 7"/><path d="M19 6a9 9 0 0 1 0 12"/>'),
  timer: icon('<path d="M9 2h6"/><path d="M12 14l3-3"/><circle cx="12" cy="14" r="8"/><path d="M17.5 7.5 19 6"/>'),
  more: icon('<circle cx="5" cy="12" r="1.3" fill="currentColor" stroke="none"/><circle cx="12" cy="12" r="1.3" fill="currentColor" stroke="none"/><circle cx="19" cy="12" r="1.3" fill="currentColor" stroke="none"/>'),
};

const views = [
  { id: "overview", label: "总览", icon: I.overview },
  { id: "media", label: "媒体", icon: I.media },
  { id: "display", label: "显示", icon: I.display },
  { id: "network", label: "网络", icon: I.network },
  { id: "system", label: "系统", icon: I.system },
];

const mediaTabs = [
  { id: "images", label: "图片" },
  { id: "videos", label: "视频/GIF" },
  { id: "tones", label: "按键音" },
];

const commonWeatherCities = ["北京", "上海", "广州", "深圳", "杭州", "成都", "重庆", "武汉", "南京", "苏州", "西安", "长沙"];
const weatherSources = {
  seniverse: {
    name: "心知天气",
    desc: "免费用户支持实时天气、三日预报和六项基础生活指数。",
    endpointLabel: "API 地址",
    endpoint: "http://api.seniverse.com",
    keyHelp: "注册心知天气，在控制台创建产品并复制 API 密钥；接口地址通常不用改。",
    keyUrl: "https://docs.seniverse.com/api/weather/now.html",
    endpointHelp: "默认 Host 是 http://api.seniverse.com，固件会自动拼接 /v3/weather/now.json、/v3/weather/daily.json 等路径。",
    guideTitle: "心知天气获取方式",
    guideSteps: [
      "打开心知天气文档或控制台，注册并登录账号。",
      "创建免费产品，在产品里复制公钥和私钥。",
      "管理台选择“心知天气”，分别填写公钥和私钥；固件会使用 HMAC-SHA1 签名。",
    ],
    keyPlaceholder: "粘贴心知天气私钥",
  },
  qweather: {
    name: "QWeather 和风天气",
    desc: "国内常用天气源，需要 API KEY；建议同时保存经纬度。",
    endpointLabel: "API Host",
    endpoint: "https://devapi.qweather.com",
    keyHelp: "进入 QWeather 项目管理创建 API KEY；官方更推荐 JWT，本固件先支持 key 参数。",
    keyUrl: "https://dev.qweather.com/docs/configuration/authentication/",
    endpointHelp: "默认 API Host 是 https://devapi.qweather.com，固件会自动调用 /v7/weather/now、/v7/weather/3d 和 /v7/indices/1d。",
    guideTitle: "QWeather 获取方式",
    guideSteps: [
      "打开 QWeather 开发平台，创建项目并添加 Web API Key。",
      "复制 API KEY，管理台选择“QWeather 和风天气”后填入 API Key。",
      "接口地址填 API Host，普通开发版通常保持 https://devapi.qweather.com。",
    ],
    keyPlaceholder: "粘贴 QWeather API KEY，留空不修改",
  },
  aliyun_72158: {
    name: "阿里云 72158 精准天气",
    desc: "阿里云市场天气商品，支持 AppCode 简单认证，也支持 AppKey/AppSecret 签名认证。",
    endpointLabel: "接口地址",
    endpoint: "https://getweather.market.alicloudapi.com/lundear/weather1d",
    keyHelp: "在阿里云市场购买/开通免费规格后，进入控制台复制 AppCode、AppKey、AppSecret；接口地址通常不用改。",
    keyUrl: "https://market.aliyun.com/detail/cmapi00072158.html",
    endpointHelp: "默认接口地址是该商品的 Host + Path。若商品页“API接口/调试”里展示的新请求地址不同，请把完整 https 地址复制到这里。",
    guideTitle: "阿里云 72158 获取方式",
    guideSteps: [
      "打开 72158 商品页，登录阿里云并开通免费规格或购买套餐。",
      "进入阿里云云市场控制台的已购买服务，复制该商品的 AppCode、AppKey、AppSecret。",
      "在商品页“API接口/调试”查看请求地址；管理台接口地址填 Host + Path 的完整 https 链接。",
      "优先填写 AppCode；如果只想用签名认证，也可以留空 AppCode，只填写 AppKey 和 AppSecret。",
    ],
    keyPlaceholder: "粘贴阿里云 AppCode，留空不修改",
    aliyun: true,
  },
  aliyun_10812: {
    name: "阿里云 10812 万维易源",
    desc: "万维易源天气服务，支持阿里云市场 AppCode 或 AppKey/AppSecret 签名认证。",
    endpointLabel: "接口地址",
    endpoint: "https://ali-weather.showapi.com/spot-to-weather",
    keyHelp: "在阿里云市场开通该商品后复制 AppCode、AppKey、AppSecret；固件会按 ShowAPI 常见结构解析返回。",
    keyUrl: "https://market.aliyun.com/detail/cmapi010812.html",
    endpointHelp: "默认接口地址是万维易源 ShowAPI 的天气路径。若商品页 API 调试给出不同 Host 或 Path，请按商品页覆盖。",
    guideTitle: "阿里云 10812 获取方式",
    guideSteps: [
      "打开 10812 商品页并开通对应规格。",
      "到阿里云云市场控制台复制该商品 AppCode、AppKey、AppSecret。",
      "在商品页“API接口/调试”复制请求地址；这里填完整接口地址，例如 https://ali-weather.showapi.com/spot-to-weather。",
      "填写 AppCode 时固件会自动添加 APPCODE 请求头；填写 AppKey/AppSecret 时会自动生成签名头。",
    ],
    keyPlaceholder: "粘贴阿里云 AppCode，留空不修改",
    aliyun: true,
  },
  aliyun_50139: {
    name: "阿里云 50139 实时多天天气",
    desc: "阿里云市场实时、多天天气与空气质量服务；支持三项云市场凭据。",
    endpointLabel: "接口地址",
    endpoint: "https://weather01.market.alicloudapi.com/weather",
    keyHelp: "在阿里云市场开通免费规格后复制 AppCode、AppKey、AppSecret；若接口文档给出专用地址，请粘贴到接口地址。",
    keyUrl: "https://market.aliyun.com/detail/cmapi00050139.html",
    endpointHelp: "默认接口地址适配 cmapi00050139。阿里云市场厂商可能调整路径，最终以商品页“API接口/调试”的 Host + Path 为准。",
    guideTitle: "阿里云 50139 获取方式",
    guideSteps: [
      "打开 50139 商品页，开通免费规格或购买套餐。",
      "在云市场控制台复制 AppCode、AppKey、AppSecret。",
      "回到商品页“API接口/调试”，复制接口请求地址并填入管理台接口地址。",
      "如果天气获取失败，优先核对三项凭据是否属于当前商品，以及接口地址是否和商品页一致。",
    ],
    keyPlaceholder: "粘贴阿里云 AppCode，留空不修改",
    aliyun: true,
  },
  aliyun_71988: {
    name: "阿里云 71988 快证天气",
    desc: "阿里云市场快证 API 天气源；支持 AppCode 或 AppKey/AppSecret。",
    endpointLabel: "接口地址",
    endpoint: "https://kzweather.market.alicloudapi.com/weather",
    keyHelp: "在阿里云市场开通该商品后复制 AppCode、AppKey、AppSecret；如商品文档接口地址不同，请在这里覆盖。",
    keyUrl: "https://market.aliyun.com/detail/cmapi00071988.html",
    endpointHelp: "默认接口地址适配 cmapi00071988。该类云市场接口经常以商品页 API 调试为准，地址变化时请手动更新。",
    guideTitle: "阿里云 71988 获取方式",
    guideSteps: [
      "打开 71988 商品页并开通服务。",
      "进入云市场控制台复制 AppCode、AppKey、AppSecret。",
      "在商品页“API接口/调试”确认请求地址，管理台接口地址填完整 https 链接。",
      "保存后固件会优先用 AppCode 访问；AppCode 留空时会用 AppKey/AppSecret 签名访问。",
    ],
    keyPlaceholder: "粘贴阿里云 AppCode，留空不修改",
    aliyun: true,
  },
};
const weatherProviderIds = Object.keys(weatherSources);
const aliyunWeatherProviderIds = weatherProviderIds.filter(id => weatherSources[id]?.aliyun);

function defaultWeatherProviderEndpoints() {
  const endpoints = {};
  weatherProviderIds.forEach(id => {
    endpoints[id] = defaultWeatherProviderEndpointFor(id);
  });
  return endpoints;
}

function defaultWeatherProviderEndpointFor(provider) {
  return (weatherSources[provider] || weatherSources.aliyun_72158).endpoint;
}

function defaultWeatherProviderKeyValues() {
  const keys = {};
  weatherProviderIds.forEach(id => {
    keys[id] = "";
  });
  return keys;
}

function defaultWeatherProviderCredentials() {
  const credentials = {};
  aliyunWeatherProviderIds.forEach(id => {
    credentials[id] = { appcode: "", appkey: "", appsecret: "" };
  });
  return credentials;
}

function isAliyunProvider(provider) {
  return !!weatherSources[provider]?.aliyun;
}

function normalizedCredential(value = {}) {
  return {
    appcode: value.appcode || value.appCode || "",
    appkey: value.appkey || value.appKey || "",
    appsecret: value.appsecret || value.appSecret || "",
  };
}

function hasOwn(obj, key) {
  return Object.prototype.hasOwnProperty.call(obj || {}, key);
}

function providerHasCredential(provider, credentials = {}, keyValue = "") {
  if (!isAliyunProvider(provider)) return !!keyValue;
  const item = normalizedCredential(credentials[provider] || {});
  return !!(item.appcode || (item.appkey && item.appsecret));
}

const state = {
  route: "overview",
  mediaTab: "images",
  info: null,
  list: null,
  appCfg: null,
  dirtyInfo: new Set(),
  dirtyAppCfg: new Set(),
  weatherKeyDrafts: {},
  weatherCredentialDrafts: {},
  weatherEndpointDrafts: {},
  online: true,
  booted: false,
  scan: { status: "idle", networks: [] },
  selectedSsid: "",
  pending: new Set(),
  weatherTest: { status: "idle", message: "" },
  upload: null,
  ffmpeg: { status: "idle", message: "" },
  videoTask: null,
  weatherDetect: { status: "idle", message: "" },
  audio: null,
  playing: "",
};

let ffmpegInstance = null;
let pollTimer = null;

function fmtSize(bytes = 0) {
  const n = Number(bytes) || 0;
  if (n >= 1024 * 1024) return `${(n / 1024 / 1024).toFixed(1)} MB`;
  if (n >= 1024) return `${Math.round(n / 1024)} KB`;
  return `${n} B`;
}

function extOf(name = "") {
  const m = String(name).toLowerCase().match(/\.[a-z0-9]+$/);
  return m ? m[0] : "";
}

function fileName(name = "") {
  return String(name).replace(/^\/+/, "");
}

function fileUrl(name = "") {
  return "/" + encodeURIComponent(fileName(name));
}

function fileLists() {
  const items = (state.list?.data || []).filter(f => f.type === "file");
  const images = items.filter(f => [".jpg", ".jpeg", ".png"].includes(extOf(f.name)));
  const videos = items.filter(f => [".mpeg", ".mpg"].includes(extOf(f.name)));
  const tones = items.filter(f => [".mp3", ".wav"].includes(extOf(f.name)));
  return { items, images, videos, tones };
}

function currentImage() {
  const { images } = fileLists();
  const selected = fileName(state.info?.jpg_file || "");
  if (state.info?.jpg_mode === "fixed" && selected) return images.find(f => fileName(f.name) === selected) || null;
  return images[0] || null;
}

async function getJSON(path) {
  const res = await fetch(path, { cache: "no-store" });
  if (!res.ok) throw new Error(`HTTP ${res.status}`);
  return res.json();
}

async function postForm(path, data = {}) {
  const res = await fetch(path, {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: new URLSearchParams(data).toString(),
  });
  if (!res.ok) throw new Error(`HTTP ${res.status}`);
  return res.text();
}

async function postPlain(path, body) {
  const res = await fetch(path, {
    method: "POST",
    headers: { "Content-Type": "text/plain" },
    body,
  });
  if (!res.ok) throw new Error(`HTTP ${res.status}`);
  return res.text();
}

function uploadFile(blob, name, onProgress) {
  return new Promise((resolve, reject) => {
    const fd = new FormData();
    fd.append("data", blob, fileName(name));
    const xhr = new XMLHttpRequest();
    xhr.open("POST", "/edit");
    xhr.upload.onprogress = ev => {
      if (ev.lengthComputable && onProgress) onProgress(ev.loaded / ev.total);
    };
    xhr.onload = () => xhr.status < 400 ? resolve() : reject(new Error(`HTTP ${xhr.status}`));
    xhr.onerror = () => reject(new Error("network"));
    xhr.send(fd);
  });
}

async function deleteFile(name) {
  const res = await fetch("/edit?path=" + encodeURIComponent("/" + fileName(name)), { method: "DELETE" });
  if (!res.ok) throw new Error(`HTTP ${res.status}`);
}

async function refreshAll(quiet = false) {
  try {
    const [info, list, appCfg] = await Promise.all([
      getJSON("/info"),
      getJSON("/list?dir=/"),
      getJSON("/config.json").catch(() => state.appCfg),
    ]);
    state.info = mergeInfoFromDevice(info);
    state.list = normalizeList(list);
    state.appCfg = mergeAppCfgFromDevice(appCfg);
    state.online = true;
    if (!quiet && !state.booted) state.booted = true;
  } catch (err) {
    state.online = false;
    if (!quiet) toast("无法连接设备，正在等待恢复", "danger");
  }
}

function normalizeInfo(info = {}) {
  return {
    mode: info.mode || "STA",
    ssid: info.ssid || "",
    ip: info.ip || "",
    theme: Number(info.theme || 0),
    space_breath_speed: clamp(Number(info.space_breath_speed ?? 1), 0, 2),
    aps_enable: !!info.aps_enable,
    weather_enable: !!info.weather_enable,    gif_enable: !!info.gif_enable,
    jpg_enable: !!info.jpg_enable,    wifi_connected: info.wifi_connected !== false,
    wifi_saved_count: Number(info.wifi_saved_count || 0),
    screen_width: Number(info.screen_width || SCREEN_W),
    screen_height: Number(info.screen_height || SCREEN_H),
    video_fit: info.video_fit === "cover" ? "cover" : "contain",
    video_audio: !!info.video_audio,
    time_roll: Number(info.time_roll || 5000),
    jpg_mode: info.jpg_mode || "roll",
    jpg_file: info.jpg_file || "",
    timezone: Number(info.timezone ?? 8),
    language: Number(info.language || 0),
    keytone: Number(info.keytone || 0),
    keytone_file: info.keytone_file || "",
    volume: clamp(Number(info.volume ?? 6), 0, 9),  };
}

function markInfoDirty(...keys) {
  keys.forEach(key => state.dirtyInfo.add(key));
}

function clearInfoDirty(...keys) {
  keys.forEach(key => state.dirtyInfo.delete(key));
}

function mergeInfoFromDevice(info) {
  const next = normalizeInfo(info);
  if (!state.info || state.dirtyInfo.size === 0) return next;
  const merged = { ...next };
  state.dirtyInfo.forEach(key => {
    if (Object.prototype.hasOwnProperty.call(state.info, key)) {
      merged[key] = state.info[key];
    }
  });
  return normalizeInfo(merged);
}

function normalizeList(list = {}) {
  return {
    size: Number(list.size || 0),
    total: Number(list.total || 1),
    data: Array.isArray(list.data) ? list.data.map(f => ({ ...f, name: fileName(f.name), size: Number(f.size || 0) })) : [],
  };
}

function collectWeatherDraftFromDom() {
  const draft = {};
  $$("[data-cfg]").forEach(input => {
    if (!input.dataset.cfg) return;
    draft[input.dataset.cfg] = input.type === "number" ? Number(input.value) : input.value;
  });
  return draft;
}

function isWeatherEditing() {
  if (state.route !== "system") return false;
  if (state.dirtyAppCfg.size > 0) return true;
  const active = document.activeElement;
  return !!(active && active.closest && active.closest(".weather-card"));
}

function normalizeAppCfg(cfg = {}) {
  const rawProvider = cfg?.weather_provider || cfg?.weatherProvider || "aliyun_72158";
  const provider = weatherSources[rawProvider] ? rawProvider : "aliyun_72158";
  const source = weatherSources[provider];
  const weatherProviderEndpoints = {
    ...defaultWeatherProviderEndpoints(),
    ...(cfg?.weather_provider_endpoints || cfg?.weatherProviderEndpoints || {}),
  };
  weatherProviderIds.forEach(id => {
    const snakeKey = `weather_endpoint_${id}`;
    if (cfg?.[snakeKey]) weatherProviderEndpoints[id] = cfg[snakeKey];
  });
  const activeEndpoint = cfg?.weather_endpoint || cfg?.weatherEndpoint || weatherProviderEndpoints[provider] || source.endpoint;
  weatherProviderEndpoints[provider] = activeEndpoint;
  const weatherProviderKeys = {
    seniverse: false,
    qweather: false,
    aliyun_72158: false,
    aliyun_10812: false,
    aliyun_50139: false,
    aliyun_71988: false,
    ...(cfg?.weather_provider_keys || cfg?.weatherProviderKeys || {}),
  };
  const weatherProviderKeyValues = {
    ...defaultWeatherProviderKeyValues(),
    ...(cfg?.weather_provider_key_values || cfg?.weatherProviderKeyValues || {}),
  };
  const weatherProviderCredentials = {
    ...defaultWeatherProviderCredentials(),
    ...(cfg?.weather_provider_credentials || cfg?.weatherProviderCredentials || {}),
  };
  aliyunWeatherProviderIds.forEach(id => {
    weatherProviderCredentials[id] = normalizedCredential(weatherProviderCredentials[id]);
  });
  weatherProviderIds.forEach(id => {
    const snakeKey = `weather_key_${id}`;
    if (hasOwn(cfg, snakeKey)) weatherProviderKeyValues[id] = cfg[snakeKey] || "";
  });
  aliyunWeatherProviderIds.forEach(id => {
    const appCodeKey = `weather_appcode_${id}`;
    const appKeyKey = `weather_appkey_${id}`;
    const appSecretKey = `weather_appsecret_${id}`;
    const appCode = hasOwn(cfg, appCodeKey) ? (cfg[appCodeKey] || "") : (weatherProviderKeyValues[id] || weatherProviderCredentials[id]?.appcode || "");
    const appKey = hasOwn(cfg, appKeyKey) ? (cfg[appKeyKey] || "") : (weatherProviderCredentials[id]?.appkey || "");
    const appSecret = hasOwn(cfg, appSecretKey) ? (cfg[appSecretKey] || "") : (weatherProviderCredentials[id]?.appsecret || "");
    weatherProviderCredentials[id] = { appcode: appCode, appkey: appKey, appsecret: appSecret };
    weatherProviderKeyValues[id] = appCode;
    weatherProviderKeys[id] = !!(weatherProviderKeys[id] || providerHasCredential(id, weatherProviderCredentials, appCode));
  });
  if (hasOwn(cfg, "weather")) weatherProviderKeyValues[provider] = cfg.weather || "";
  if (isAliyunProvider(provider)) {
    const activeCredential = normalizedCredential(weatherProviderCredentials[provider]);
    if (hasOwn(cfg, "weather_appcode")) activeCredential.appcode = cfg.weather_appcode || "";
    if (hasOwn(cfg, "weather_appkey")) activeCredential.appkey = cfg.weather_appkey || "";
    if (hasOwn(cfg, "weather_appsecret")) activeCredential.appsecret = cfg.weather_appsecret || "";
    if (hasOwn(cfg, "weather")) activeCredential.appcode = cfg.weather || "";
    weatherProviderCredentials[provider] = activeCredential;
    weatherProviderKeyValues[provider] = activeCredential.appcode;
    weatherProviderKeys[provider] = !!(weatherProviderKeys[provider] || providerHasCredential(provider, weatherProviderCredentials, activeCredential.appcode));
  }
  return {
    weatherConfigured: !!(cfg?.weather_configured || cfg?.weatherConfigured || cfg?.weather || weatherProviderKeys[provider] || providerHasCredential(provider, weatherProviderCredentials, weatherProviderKeyValues[provider])),
    weatherProviderKeys,
    weatherProviderKeyValues,
    weatherProviderCredentials,
    weatherProviderEndpoints,
    weatherPublicKeySeniverse: cfg?.weather_publickey_seniverse || cfg?.weatherPublicKeySeniverse || (provider === "seniverse" ? (cfg?.weather_publickey || "") : ""),
    weatherProvider: provider,
    weatherEndpoint: activeEndpoint,
    weatherLat: cfg?.weather_lat || cfg?.weatherLat || "",
    weatherLon: cfg?.weather_lon || cfg?.weatherLon || "",
    city: cfg?.city || "",
  };
}

function mergeAppCfgFromDevice(cfg) {
  const next = normalizeAppCfg(cfg);
  if (isWeatherEditing()) {
    return normalizeAppCfg({
      ...next,
      ...state.appCfg,
      ...collectWeatherDraftFromDom(),
    });
  }
  if (!state.appCfg || state.dirtyAppCfg.size === 0) return next;
  const merged = { ...next };
  state.dirtyAppCfg.forEach(key => {
    if (Object.prototype.hasOwnProperty.call(state.appCfg, key)) {
      merged[key] = state.appCfg[key];
    }
  });
  return normalizeAppCfg(merged);
}

function toast(message, type = "ok", timeout = 2600) {
  const el = document.createElement("div");
  el.className = `toast toast-${type}`;
  el.innerHTML = `<span>${esc(message)}</span>`;
  toasts.appendChild(el);
  window.setTimeout(() => {
    el.classList.add("is-leaving");
    window.setTimeout(() => el.remove(), 180);
  }, timeout);
}

function modal(html, options = {}) {
  const overlay = document.createElement("div");
  overlay.className = "modal-layer";
  overlay.innerHTML = `<section class="modal-card ${options.wide ? "modal-wide" : ""}" role="dialog" aria-modal="true">${html}</section>`;
  modals.appendChild(overlay);
  const close = () => overlay.remove();
  overlay.addEventListener("click", ev => {
    if (ev.target === overlay) close();
  });
  document.addEventListener("keydown", function onKey(ev) {
    if (ev.key === "Escape") {
      close();
      document.removeEventListener("keydown", onKey);
    }
  });
  return { overlay, card: $(".modal-card", overlay), close };
}

function confirmDialog({ title, body, ok = "确定", danger = false }) {
  return new Promise(resolve => {
    const m = modal(`
      <h2>${esc(title)}</h2>
      <p class="modal-copy">${esc(body)}</p>
      <div class="button-row">
        <button class="btn ghost" data-cancel>取消</button>
        <button class="btn ${danger ? "danger" : "primary"}" data-ok>${esc(ok)}</button>
      </div>
    `);
    $("[data-cancel]", m.card).onclick = () => { m.close(); resolve(false); };
    $("[data-ok]", m.card).onclick = () => { m.close(); resolve(true); };
  });
}

function setPending(key, value) {
  if (value) state.pending.add(key);
  else state.pending.delete(key);
  document.body.toggleAttribute("data-busy", state.pending.size > 0);
}

async function runAction(key, fn, okMessage = "已保存") {
  setPending(key, true);
  render();
  try {
    await fn();
    await refreshAll(true);
    toast(okMessage);
    return true;
  } catch (err) {
    toast("操作失败，已回滚", "danger");
    await refreshAll(true);
    return false;
  } finally {
    setPending(key, false);
    render();
  }
}

function busy(key) {
  return state.pending.has(key);
}

function isPortal() {
  return location.pathname === "/wifi";
}

function routeFromHash() {
  const id = (location.hash.replace(/^#\/?/, "") || "overview").split("/")[0];
  return views.some(v => v.id === id) ? id : "overview";
}

function resetPageScroll() {
  requestAnimationFrame(() => {
    window.scrollTo({ top: 0, left: 0, behavior: "auto" });
  });
}

function go(id) {
  if (state.route === id && location.hash === "#/" + id) {
    resetPageScroll();
    return;
  }
  location.hash = "#/" + id;
}

function appShell() {
  const info = state.info;
  const list = state.list;
  const storagePct = list ? clamp(Math.round(list.size / Math.max(1, list.total) * 100), 0, 100) : 0;
  const nav = views.map(v => `
    <button class="nav-item ${state.route === v.id ? "active" : ""}" data-route="${v.id}" aria-label="${v.label}">
      ${v.icon}<span>${v.label}</span>
    </button>
  `).join("");

  return `
    <aside class="sidebar glass-shell">
      <div class="brand-lockup">${I.logo}<div><b>SKYLOONG</b><small>Screen Console</small></div></div>
      <nav class="side-nav">${nav}</nav>
      <div class="sidebar-meter">
        <div class="meter-head"><span>存储</span><b>${storagePct}%</b></div>
        <div class="meter"><span style="width:${storagePct}%"></span></div>
        <small>${list ? `${fmtSize(list.size)} / ${fmtSize(list.total)}` : "读取中"}</small>
      </div>
    </aside>

    <header class="topbar glass-shell">
      <button class="icon-button mobile-only" data-route="system" aria-label="系统">${I.more}</button>
      <div class="device-chip">
        <span class="live-dot ${state.online ? "online" : "offline"}"></span>
        <div><b>${info ? esc(info.ssid || "未连接") : "连接中"}</b><small>${info ? `${esc(info.mode)} · ${esc(info.ip)}` : "读取设备状态"}</small></div>
      </div>
      <div class="top-actions">
        <button class="btn subtle" data-sync-time>${I.clock}<span>同步时间</span></button>
        <button class="btn subtle danger-text desktop-only" data-reboot>${I.power}<span>重启</span></button>
      </div>
    </header>

    <main class="content" id="content">${pageContent()}</main>

    <nav class="mobile-nav glass-shell">${nav}</nav>
    ${state.online ? "" : `<div class="offline-banner">设备暂时离线，页面会自动重试</div>`}
  `;
}

function pageContent() {
  if (!state.info || !state.list) return skeletonPage();
  switch (state.route) {
    case "media": return viewMedia();
    case "display": return viewDisplay();
    case "network": return viewNetwork();
    case "system": return viewSystem();
    default: return viewOverview();
  }
}

function render() {
  if (isPortal()) {
    app.innerHTML = portalView();
    bindPortal();
    return;
  }
  app.innerHTML = appShell();
  bindCommon();
  bindView();
}

function skeletonPage() {
  return `
    <div class="page-title"><p>SKYLOONG 4.0</p><h1>正在连接屏幕</h1></div>
    <div class="grid two">
      <section class="panel"><div class="skel title"></div><div class="skel block"></div></section>
      <section class="panel"><div class="skel title"></div><div class="skel block"></div></section>
    </div>
  `;
}

function viewOverview() {
  const info = state.info;
  const list = state.list;
  const files = fileLists();
  const img = currentImage();
  const storagePct = clamp(Math.round(list.size / Math.max(1, list.total) * 100), 0, 100);
  const rollSec = Math.round(info.time_roll / 1000);
  return `
    <div class="page-title">
      <p>设备总览</p>
      <h1>把屏幕内容、网络和素材放在一个工作台里</h1>
    </div>

    <section class="hero-grid">
      <div class="screen-panel">
        <div class="screen-frame">
          <div class="screen-canvas">
            ${img ? `<img src="${fileUrl(img.name)}" alt="当前屏幕图片">` : `<div class="screen-empty">${I.logo}<b>320 × 240</b><span>暂无图片</span></div>`}
          </div>
        </div>
        <div class="screen-caption">
          <div><b>${info.jpg_enable ? (info.jpg_mode === "fixed" ? "固定显示" : `轮播 ${rollSec}s`) : "图片显示关闭"}</b><small>${img ? esc(img.name) : "上传图片后可设为当前"}</small></div>
          <button class="btn primary" data-route="media">${I.upload}<span>管理素材</span></button>
        </div>
      </div>

      <div class="quick-panel panel">
        <div class="panel-head"><span>快捷操作</span><small>常用动作</small></div>
        <button class="quick-action" data-route="network">${I.network}<span><b>切换 Wi-Fi</b><small>${esc(info.ip || "查看当前网络")}</small></span></button>
        <button class="quick-action" data-route="media" data-media-tab="images">${I.image}<span><b>上传图片</b><small>裁剪到屏幕比例</small></span></button>
        <button class="quick-action" data-route="media" data-media-tab="videos">${I.media}<span><b>上传视频/GIF</b><small>${info.video_fit === "cover" ? "铺满屏幕" : "完整显示"}</small></span></button>
        <button class="quick-action danger-hover" data-reboot>${I.power}<span><b>重启设备</b><small>约 10 到 20 秒恢复</small></span></button>
      </div>
    </section>

    <div class="metric-grid">
      ${metricCard("连接", info.mode, info.ssid || "SKYLOONG 4.0 Screen", state.online ? "good" : "bad")}
      ${metricCard("存储", `${storagePct}%`, `${fmtSize(list.size)} / ${fmtSize(list.total)}`, storagePct > 88 ? "warn" : "good")}
      ${metricCard("素材", `${files.images.length + files.videos.length + files.tones.length}`, `${files.images.length} 图片 · ${files.videos.length} 视频 · ${files.tones.length} 音效`, "neutral")}
      ${metricCard("已存 Wi-Fi", String(info.wifi_saved_count || 0), info.wifi_connected ? "自动联网已启用" : "当前未联网", info.wifi_connected ? "good" : "warn")}
    </div>

    <section class="panel">
      <div class="panel-head"><span>小应用</span><small>直接开关屏幕功能</small></div>
      <div class="app-switches">
        ${appToggle("aps", "手速监测", info.aps_enable)}
        ${appToggle("weather", "本地天气", info.weather_enable)}        ${appToggle("gif", "视频/GIF", info.gif_enable)}
        ${appToggle("jpg", "图片相册", info.jpg_enable)}
      </div>
    </section>
  `;
}

function metricCard(label, value, hint, tone) {
  return `<section class="metric-card ${tone}"><span>${esc(label)}</span><b>${esc(value)}</b><small>${esc(hint)}</small></section>`;
}

function appToggle(appId, label, checked) {
  return `
    <label class="toggle-row">
      <span><b>${esc(label)}</b><small>${checked ? "运行中" : "已关闭"}</small></span>
      <input type="checkbox" data-app-toggle="${appId}" ${checked ? "checked" : ""} ${busy(`app-${appId}`) ? "disabled" : ""}>
      <i></i>
    </label>
  `;
}

function viewMedia() {
  const tab = state.mediaTab;
  return `
    <div class="page-title row-title">
      <div><p>媒体库</p><h1>素材上传、转码和当前显示</h1></div>
      <div class="segmented">
        ${mediaTabs.map(t => `<button class="${tab === t.id ? "active" : ""}" data-media-tab="${t.id}">${t.label}</button>`).join("")}
      </div>
    </div>
    ${tab === "images" ? mediaImages() : tab === "videos" ? mediaVideos() : mediaTones()}
  `;
}

function mediaImages() {
  const { images } = fileLists();
  const current = fileName(state.info.jpg_file);
  return `
    <div class="grid media-layout">
      <section class="panel upload-panel">
        <div class="panel-head"><span>上传图片</span><small>输出 ${SCREEN_W} × ${SCREEN_H}</small></div>
        ${dropzone("image", "选择或拖入图片", "支持 JPG、PNG，上传前可裁剪", "image/*")}
      </section>
      <section class="panel">
        <div class="panel-head"><span>图片库</span><small>${images.length} 个文件</small></div>
        ${images.length ? `<div class="media-grid">${images.map(f => imageTile(f, current)).join("")}</div>` : emptyState(I.image, "还没有图片", "上传一张作为屏幕主图")}
      </section>
    </div>
  `;
}

function imageTile(file, current) {
  const name = fileName(file.name);
  const inUse = current === name;
  return `
    <article class="image-tile" data-file="${esc(name)}">
      <img src="${fileUrl(name)}" alt="${esc(name)}">
      <div>
        <b title="${esc(name)}">${esc(name)}</b>
        <small>${fmtSize(file.size)}${inUse ? " · 当前" : ""}</small>
      </div>
      <div class="tile-actions">
        <button class="btn mini ${inUse ? "primary" : "subtle"}" data-use-image="${esc(name)}">${inUse ? I.check : I.display}<span>${inUse ? "当前" : "显示"}</span></button>
        <button class="icon-button danger-text" data-delete="${esc(name)}" aria-label="删除 ${esc(name)}">${I.trash}</button>
      </div>
    </article>
  `;
}

function mediaVideos() {
  const { videos } = fileLists();
  const fit = state.info.video_fit || "contain";
  const audio = !!state.info.video_audio;
  const task = state.videoTask;
  return `
    <div class="grid media-layout">
      <section class="panel upload-panel">
        <div class="panel-head"><span>上传视频/GIF</span><small>${fit === "cover" ? "铺满裁切" : "完整留边"}</small></div>
        <div class="fit-choice">
          <button class="${fit === "contain" ? "active" : ""}" data-video-fit="contain">完整显示</button>
          <button class="${fit === "cover" ? "active" : ""}" data-video-fit="cover">铺满屏幕</button>
        </div>
        <div class="inline-note" data-video-tradeoff-note>
          ${fit === "cover" ? "铺满会裁切画面边缘，但会占满 320 × 240 屏幕。" : "完整显示会保留全部画面，比例不一致时会补黑边。"}关闭声音会更流畅，适合 4K 原片或复杂动态画面。
        </div>
        <label class="toggle-row wide-toggle">
          <span><b>播放视频声音</b><small>${audio ? "上传时保留低码率单声道音轨" : "上传时移除音轨，优先保证流畅"}</small></span>
          <input type="checkbox" data-video-audio ${audio ? "checked" : ""}>
          <i></i>
        </label>
        ${dropzone("video", "选择或拖入 MP4/GIF/MPEG", "MP4/GIF 会在浏览器内转成屏幕 MPEG", "video/mp4,image/gif,.mpeg")}
        ${state.ffmpeg.status === "loading" ? progressLine("转码器加载中", 40, "首次会稍慢") : ""}
        ${state.ffmpeg.status === "error" ? `<div class="inline-error">转码器加载失败，请检查电脑网络后重试 <button class="link-btn" data-retry-ffmpeg>重试</button></div>` : ""}
        ${task ? videoTaskHTML(task) : ""}
      </section>
      <section class="panel">
        <div class="panel-head"><span>视频库</span><small>${videos.length} 个文件</small></div>
        ${videos.length ? `<div class="file-list">${videos.map(f => fileRow(f, I.media)).join("")}</div>` : emptyState(I.media, "还没有视频", "上传 MP4 或 GIF 后播放")}
      </section>
    </div>
  `;
}

function mediaTones() {  return `
    <div class="grid media-layout">
      <section class="panel upload-panel">
        <div class="panel-head"><span>上传按键音</span><small>MP3 或 WAV</small></div>
        <div class="tone-choice">
          ${toneButton(0, "关闭")}
          ${toneButton(1, "内置 1")}
          ${toneButton(2, "内置 2")}
          ${toneButton(3, "内置 3")}
          ${toneButton(5, "弹跳三连")}
          ${toneButton(6, "篮球律动")}
          ${toneButton(7, "趣味上扬")}
          ${toneButton(4, "自定义")}
        </div>
        ${dropzone("tone", "选择或拖入音频", "上传后可设为自定义按键音", "audio/mpeg,audio/wav,.mp3,.wav")}
      </section>
      <section class="panel">
        <div class="panel-head"><span>音效库</span><small>${tones.length} 个文件</small></div>
        ${tones.length ? `<div class="file-list">${tones.map(f => toneRow(f)).join("")}</div>` : emptyState(I.music, "还没有自定义音效", "上传后可以试听和选用")}
      </section>
    </div>
  `;
}

function toneButton(value, label) {
  return `<button class="${state.info.keytone === value ? "active" : ""}" data-keytone="${value}">${esc(label)}</button>`;
}

function toneRow(file) {
  const name = fileName(file.name);
  const inUse = state.info.keytone === 4 && fileName(state.info.keytone_file) === name;
  return `
    <article class="file-row" data-file="${esc(name)}">
      <span class="file-icon">${I.music}</span>
      <div><b>${esc(name)}</b><small>${fmtSize(file.size)}${inUse ? " · 在用" : ""}</small></div>
      <div class="row-actions">
        <button class="icon-button" data-play-tone="${esc(name)}" aria-label="试听 ${esc(name)}">${state.playing === name ? I.stop : I.play}</button>
        <button class="btn mini ${inUse ? "primary" : "subtle"}" data-use-tone="${esc(name)}">${inUse ? "在用" : "选用"}</button>
        <button class="icon-button danger-text" data-delete="${esc(name)}" aria-label="删除 ${esc(name)}">${I.trash}</button>
      </div>
    </article>
  `;
}

function fileRow(file, iconSvg) {
  const name = fileName(file.name);
  return `
    <article class="file-row" data-file="${esc(name)}">
      <span class="file-icon">${iconSvg}</span>
      <div><b>${esc(name)}</b><small>${fmtSize(file.size)}</small></div>
      <div class="row-actions">
        <button class="icon-button danger-text" data-delete="${esc(name)}" aria-label="删除 ${esc(name)}">${I.trash}</button>
      </div>
    </article>
  `;
}

function dropzone(type, title, hint, accept) {
  return `
    <div class="dropzone" data-dropzone="${type}" tabindex="0">
      ${I.upload}
      <b>${esc(title)}</b>
      <small>${esc(hint)}</small>
    </div>
    <input class="sr-only" type="file" data-file-input="${type}" accept="${esc(accept)}">
  `;
}

function videoTaskHTML(task) {
  const transcode = task.stage === "upload" ? 100 : task.transcode;
  const upload = task.stage === "upload" ? task.upload : 0;
  return `
    <div class="task-card">
      <div class="task-head"><b>${esc(task.name)}</b><button class="btn mini subtle" data-cancel-video>取消</button></div>
      ${progressLine("转码", transcode, task.stage === "transcode" ? "处理中" : "完成")}
      ${progressLine("上传", upload, task.stage === "upload" ? "写入设备" : "等待转码")}
      ${task.error ? `<div class="inline-error">${esc(task.error)}</div>` : ""}
    </div>
  `;
}

function progressLine(label, pct, hint = "") {
  return `
    <div class="progress-line">
      <div><span>${esc(label)}</span><small>${esc(hint)}</small></div>
      <div class="progress"><i style="width:${clamp(Math.round(pct), 0, 100)}%"></i></div>
      <b>${clamp(Math.round(pct), 0, 100)}%</b>
    </div>
  `;
}

function emptyState(iconSvg, title, copy) {
  return `<div class="empty-state">${iconSvg}<b>${esc(title)}</b><small>${esc(copy)}</small></div>`;
}

function viewDisplay() {
  const info = state.info;
  const { images } = fileLists();
  return `
    <div class="page-title">
      <p>显示控制</p>
      <h1>决定屏幕显示什么，以及素材如何适配</h1>
    </div>
    <div class="grid two">
      <section class="panel">
        <div class="panel-head"><span>主题</span><small>屏幕主界面</small></div>
        <div class="theme-grid">
          ${Array.from({ length: 4 }, (_, i) => `
            <button class="theme-card ${info.theme === i ? "active" : ""}" data-theme="${i}" aria-label="主题 ${i + 1}${i === 3 ? `，${spaceBreathLabels[info.space_breath_speed]}呼吸` : ""}">
              <img src="/theme${i + 1}.png?v=theme-layout-20260709d" alt="主题 ${i + 1}">
              ${i === 3 ? `<small class="theme-speed">${spaceBreathLabels[info.space_breath_speed]}</small>` : ""}
              ${info.theme === i ? `<span>${I.check}</span>` : ""}
            </button>
          `).join("")}
        </div>
      </section>
      <section class="panel">
        <div class="panel-head"><span>图片相册</span><small>${images.length} 张图片</small></div>
        <div class="field-stack">
          <label class="toggle-row">
            <span><b>启用图片显示</b><small>${info.jpg_enable ? "已启用" : "已关闭"}</small></span>
            <input type="checkbox" data-app-toggle="jpg" ${info.jpg_enable ? "checked" : ""}>
            <i></i>
          </label>
          <label class="field">
            <span>显示模式</span>
            <select data-jpg-mode>
              <option value="roll" ${info.jpg_mode === "roll" ? "selected" : ""}>自动轮播</option>
              <option value="fixed" ${info.jpg_mode === "fixed" ? "selected" : ""}>固定图片</option>
            </select>
          </label>
          <label class="field">
            <span>固定显示图片</span>
            <select data-jpg-file>
              <option value="">未选择</option>
              ${images.map(f => `<option value="${esc(fileName(f.name))}" ${fileName(info.jpg_file) === fileName(f.name) ? "selected" : ""}>${esc(fileName(f.name))}</option>`).join("")}
            </select>
          </label>
          <label class="field">
            <span>轮播间隔 <b>${Math.round(info.time_roll / 1000)}s</b></span>
            <input type="range" min="2" max="30" step="1" value="${Math.round(info.time_roll / 1000)}" data-time-roll>
          </label>
        </div>
      </section>
      <section class="panel span-two">
        <div class="panel-head"><span>视频/GIF 适配</span><small>${info.screen_width} × ${info.screen_height}</small></div>
        <div class="field-stack">
          <div class="fit-cards">
            <button class="${info.video_fit === "contain" ? "active" : ""}" data-video-fit="contain">
              <b>完整显示</b><small>保留完整画面，空余区域补黑</small>
            </button>
            <button class="${info.video_fit === "cover" ? "active" : ""}" data-video-fit="cover">
              <b>铺满屏幕</b><small>填满屏幕，必要时裁掉边缘</small>
            </button>
          </div>
          <label class="toggle-row wide-toggle">
            <span><b>播放视频声音</b><small>${info.video_audio ? "视频带 MP2 音轨时会从屏幕扬声器播放" : "视频保持静音播放"}</small></span>
            <input type="checkbox" data-video-audio ${info.video_audio ? "checked" : ""}>
            <i></i>
          </label>
        </div>
      </section>
    </div>
  `;
}

function viewNetwork() {
  const info = state.info;
  return `
    <div class="page-title row-title">
      <div><p>网络</p><h1>扫描、保存并切换 Wi-Fi</h1></div>
      <button class="btn subtle" data-scan ${state.scan.status === "scanning" ? "disabled" : ""}>${I.refresh}<span>${state.scan.status === "scanning" ? "扫描中" : "重新扫描"}</span></button>
    </div>
    <div class="grid network-layout">
      <section class="panel">
        <div class="panel-head"><span>当前连接</span><small>${info.wifi_connected ? "在线" : "未联网"}</small></div>
        <div class="connection-card">
          <span class="connection-icon">${I.network}</span>
          <div><b>${esc(info.ssid || "未连接")}</b><small>${esc(info.mode)} · ${esc(info.ip || "无 IP")}</small></div>
        </div>
        <div class="metric-mini">
          <span>已保存网络</span><b>${info.wifi_saved_count || 0}</b>
        </div>
      </section>
      <section class="panel">
        <div class="panel-head"><span>可用 Wi-Fi</span><small>${state.scan.status === "done" ? `${state.scan.networks.length} 个` : "点击扫描"}</small></div>
        ${networkList()}
      </section>
      <section class="panel span-two">
        <div class="panel-head"><span>连接网络</span><small>设备可能切换到新的 IP</small></div>
        <div class="network-form">
          <label class="field"><span>SSID</span><input data-wifi-ssid value="${esc(state.selectedSsid)}" placeholder="输入网络名称"></label>
          <label class="field password-field"><span>密码</span><input data-wifi-password type="password" autocomplete="off" placeholder="开放网络可留空"><button class="icon-button" data-toggle-password aria-label="显示密码">${I.eye}</button></label>
          <button class="btn primary" data-connect-wifi>${I.network}<span>连接此网络</span></button>
        </div>
      </section>
    </div>
  `;
}

function networkList() {
  if (state.scan.status === "idle") return emptyState(I.network, "还未扫描", "点击右上角重新扫描");
  if (state.scan.status === "scanning") return `<div class="scan-skeleton">${progressLine("扫描附近网络", 55, "请稍候")}</div>`;
  if (!state.scan.networks.length) return emptyState(I.network, "没有发现网络", "可以手动输入 SSID");
  return `
    <div class="network-list">
      ${state.scan.networks.map(n => `
        <button class="network-row ${state.selectedSsid === n.ssid ? "active" : ""}" data-pick-ssid="${esc(n.ssid)}">
          <span class="wifi-bars" data-level="${rssiLevel(n.rssi)}"><i></i><i></i><i></i><i></i></span>
          <b>${esc(n.ssid)}</b>
          <small>${n.rssi} dBm</small>
          ${I.lock}
        </button>
      `).join("")}
    </div>
  `;
}

function rssiLevel(rssi) {
  return rssi > -55 ? 4 : rssi > -68 ? 3 : rssi > -80 ? 2 : 1;
}

function viewSystem() {
  const info = state.info;
  const cfg = state.appCfg || normalizeAppCfg();
  const { tones } = fileLists();
  return `
    <div class="page-title">
      <p>系统设置</p>
      <h1>语言、时间、应用参数和设备维护</h1>
    </div>
    <div class="grid two">
      <section class="panel">
        <div class="panel-head"><span>时间与语言</span><small>设备屏幕</small></div>
        <div class="field-stack">
          <label class="field"><span>时区 UTC${info.timezone >= 0 ? "+" : ""}${info.timezone}</span><input type="range" min="-12" max="12" step="1" value="${info.timezone}" data-timezone></label>
          <div class="segmented wide">
            <button class="${info.language === 0 ? "active" : ""}" data-language="0">中文</button>
            <button class="${info.language === 1 ? "active" : ""}" data-language="1">English</button>
          </div>
          <button class="btn subtle" data-sync-time>${I.clock}<span>用本机时间校准</span></button>
        </div>
      </section>
      <section class="panel">
        <div class="panel-head"><span>设备声音</span><small>扬声器输出</small></div>
        <div class="field-stack">
          <label class="field range-field">
            <span>音量 <b class="range-value">${info.volume}</b></span>
            <input type="range" min="0" max="9" step="1" value="${info.volume}" data-volume aria-label="设备音量">
            <small>0 为静音，9 为最大音量。按键音和屏幕提示音都会使用这个音量。</small>
          </label>
          <label class="toggle-row wide-toggle">
            <span><b>播放视频声音</b><small>${info.video_audio ? "视频带音轨时会播放声音" : "视频静音播放"}</small></span>
            <input type="checkbox" data-video-audio ${info.video_audio ? "checked" : ""}>
            <i></i>
          </label>
        </div>
      </section>
      <section class="panel span-two">
        <div class="panel-head"><span>天气服务</span><small>城市和数据源</small></div>
        <div class="weather-layout">
          ${weatherSettingsCard(cfg)}
          ${weatherConsoleDocs()}
        </div>
        <div class="button-row weather-actions">
          <button class="btn subtle" data-test-weather ${busy("weather-test") ? "disabled" : ""}>${I.refresh}<span>${busy("weather-test") ? "测试中" : "测试当前天气源"}</span></button>
          <button class="btn primary" data-save-appcfg>保存天气参数</button>
        </div>
      </section>
      <section class="panel span-two danger-zone">
        <div class="panel-head"><span>设备维护</span><small>谨慎操作</small></div>
        <button class="btn danger" data-reboot>${I.power}<span>重启设备</span></button>
      </section>
    </div>
  `;
}

function weatherSettingsCard(cfg) {
  const source = weatherSources[cfg.weatherProvider] || weatherSources.aliyun_72158;
  const keyDraft = Object.prototype.hasOwnProperty.call(state.weatherKeyDrafts, cfg.weatherProvider)
    ? state.weatherKeyDrafts[cfg.weatherProvider]
    : (cfg.weatherProviderKeyValues?.[cfg.weatherProvider] || "");
  const credentialDraft = {
    ...normalizedCredential(cfg.weatherProviderCredentials?.[cfg.weatherProvider] || {}),
    ...normalizedCredential(state.weatherCredentialDrafts?.[cfg.weatherProvider] || {}),
  };
  const endpointDraft = Object.prototype.hasOwnProperty.call(state.weatherEndpointDrafts, cfg.weatherProvider)
    ? state.weatherEndpointDrafts[cfg.weatherProvider]
    : cfg.weatherEndpoint;
  const keyPlaceholder = source.keyPlaceholder;
  return `
    <div class="weather-card">
            <div class="weather-card-head">
        <div><span>天气服务</span><b>${esc(source.name)}</b></div>
              <button class="btn subtle" data-detect-city ${state.weatherDetect.status === "detecting" ? "disabled" : ""}>${I.location}<span>${state.weatherDetect.status === "detecting" ? "识别中" : "自动识别"}</span></button>
            </div>
      <label class="field">
        <span>天气源</span>
        <select data-cfg="weather_provider" data-weather-provider>
          ${Object.entries(weatherSources).map(([id, item]) => `<option value="${esc(id)}" ${cfg.weatherProvider === id ? "selected" : ""}>${esc(item.name)}</option>`).join("")}
        </select>
        <small>${esc(source.desc)}</small>
      </label>
      <label class="field">
        <span>${esc(source.endpointLabel)}</span>
        <input data-cfg="weather_endpoint" value="${esc(endpointDraft || source.endpoint)}" placeholder="${esc(source.endpoint)}">
        <small>${esc(source.endpointHelp)}</small>
      </label>
      ${source.aliyun ? aliyunCredentialFields(source, credentialDraft) : cfg.weatherProvider === "seniverse" ? seniverseCredentialFields(cfg, keyDraft) : `
        <label class="field">
          <span>API Key</span>
          <input data-cfg="weather" type="text" autocomplete="off" value="${esc(keyDraft)}" placeholder="${esc(keyPlaceholder)}">
          <small>${esc(source.keyHelp)} 当前天气源的 Key 会明文显示在这里，切换天气源也会保留各自保存的值。 <a href="${esc(source.keyUrl)}" target="_blank" rel="noreferrer">查看获取方法</a></small>
        </label>
      `}
      ${weatherSourceGuide(source)}
            <label class="field"><span>手动选择或输入</span><input data-cfg="city" list="weather-city-list" value="${esc(cfg.city)}" placeholder="例如 北京"></label>
            <datalist id="weather-city-list">${commonWeatherCities.map(city => `<option value="${esc(city)}"></option>`).join("")}</datalist>
            <div class="city-chips">${commonWeatherCities.slice(0, 8).map(city => `<button class="${cfg.city === city ? "active" : ""}" data-city-pick="${esc(city)}">${esc(city)}</button>`).join("")}</div>
      <div class="mini-grid">
        <label class="field"><span>纬度</span><input data-cfg="weather_lat" inputmode="decimal" value="${esc(cfg.weatherLat)}" placeholder="39.9042"></label>
        <label class="field"><span>经度</span><input data-cfg="weather_lon" inputmode="decimal" value="${esc(cfg.weatherLon)}" placeholder="116.4074"></label>
          </div>
      ${state.weatherTest.message ? `<div class="weather-test-result ${state.weatherTest.status === "ok" ? "ok" : state.weatherTest.status === "error" ? "error" : ""}">${esc(state.weatherTest.message)}</div>` : ""}
      <small>${state.weatherDetect.message ? esc(state.weatherDetect.message) : weatherStatusText(cfg)}</small>
    </div>
  `;
}

function seniverseCredentialFields(cfg, privateKey) {
  return `
    <div class="mini-grid">
      <label class="field">
        <span>公钥</span>
        <input data-cfg="weather_publickey" data-weather-seniverse="publickey" type="text" autocomplete="off" value="${esc(cfg.weatherPublicKeySeniverse || "")}" placeholder="粘贴心知天气公钥">
        <small>公钥会出现在请求签名参数中，可明文显示。</small>
      </label>
      <label class="field">
        <span>私钥</span>
        <input data-cfg="weather" data-weather-seniverse="privatekey" type="text" autocomplete="off" value="${esc(privateKey)}" placeholder="粘贴心知天气私钥">
        <small>私钥用于 HMAC-SHA1 签名；免费用户使用中文、摄氏度、三日预报和基础生活指数。</small>
      </label>
    </div>
  `;
}
function aliyunCredentialFields(source, credential) {
  return `
    <div class="mini-grid">
      <label class="field">
        <span>AppCode</span>
        <input data-cfg="weather_appcode" data-weather-credential="appcode" type="text" autocomplete="off" value="${esc(credential.appcode)}" placeholder="${esc(source.keyPlaceholder)}">
        <small>简单认证用 AppCode；固件会自动添加 APPCODE 请求头。</small>
      </label>
      <label class="field">
        <span>AppKey</span>
        <input data-cfg="weather_appkey" data-weather-credential="appkey" type="text" autocomplete="off" value="${esc(credential.appkey)}" placeholder="粘贴阿里云 AppKey">
        <small>签名认证用 AppKey；AppCode 留空时会使用它。</small>
      </label>
    </div>
    <label class="field">
      <span>AppSecret</span>
      <input data-cfg="weather_appsecret" data-weather-credential="appsecret" type="text" autocomplete="off" value="${esc(credential.appsecret)}" placeholder="粘贴阿里云 AppSecret">
      <small>${esc(source.keyHelp)} 三项凭据会按天气源单独保存。 <a href="${esc(source.keyUrl)}" target="_blank" rel="noreferrer">查看获取方法</a></small>
    </label>
  `;
}

function weatherConsoleDocs() {
  const aliyunItems = [
    ["阿里云 72158", "https://market.aliyun.com/detail/cmapi00072158.html", weatherSources.aliyun_72158.endpoint],
    ["阿里云 10812", "https://market.aliyun.com/detail/cmapi010812.html", weatherSources.aliyun_10812.endpoint],
    ["阿里云 50139", "https://market.aliyun.com/detail/cmapi00050139.html", weatherSources.aliyun_50139.endpoint],
    ["阿里云 71988", "https://market.aliyun.com/detail/cmapi00071988.html", weatherSources.aliyun_71988.endpoint],
  ];
  return `
    <div class="weather-docs">
      <div class="weather-guide-title">
        <b>天气源获取教程</b>
        <span>后台直接看，不用翻 README</span>
      </div>
      <div class="weather-docs-steps">
        <b>阿里云三项凭据怎么填</b>
        <ol>
          <li>打开下面对应商品页，登录阿里云，开通免费规格或购买套餐。</li>
          <li>进入阿里云云市场控制台，在已购买服务里复制该商品的 AppCode、AppKey、AppSecret。</li>
          <li>回到商品页的 API接口 / API调试 / 接口文档 区域，复制请求地址的 Host + Path。</li>
          <li>把 Host + Path 的完整 https 链接填进接口地址；AppCode 和 AppKey/AppSecret 可以同时保存。</li>
        </ol>
      </div>
      <div class="weather-docs-list">
        ${aliyunItems.map(([name, url, endpoint]) => `
          <a href="${esc(url)}" target="_blank" rel="noreferrer">
            <b>${esc(name)}</b>
            <span>${esc(endpoint)}</span>
          </a>
        `).join("")}
      </div>
      <div class="weather-docs-steps">
        <b>心知 / QWeather</b>
        <ol>
          <li>心知天气：打开文档或控制台，创建产品，复制私钥 Key，接口地址保持 http://api.seniverse.com。</li>
          <li>QWeather：开发平台创建项目，复制 Web API KEY，接口地址保持 https://devapi.qweather.com。</li>
        </ol>
      </div>
    </div>
  `;
}

function weatherSourceGuide(source) {
  const steps = source.guideSteps || [];
  if (!steps.length) return "";
  return `
    <div class="weather-guide">
      <div class="weather-guide-title">
        <b>${esc(source.guideTitle || "获取方式")}</b>
        <a href="${esc(source.keyUrl)}" target="_blank" rel="noreferrer">打开链接</a>
      </div>
      <ol>
        ${steps.map(step => `<li>${esc(step)}</li>`).join("")}
      </ol>
    </div>
  `;
}

function weatherStatusText(cfg) {
  const hasSavedKey = !!cfg.weatherProviderKeys?.[cfg.weatherProvider];
  if (hasSavedKey) return "该天气源的凭据已保存；接口地址也会按天气源单独记住，切换回来不需要重复填写。";
  return "请选择天气源并填入自己的 API Key；阿里云源请填 AppCode、AppKey、AppSecret，接口地址以商品页 API接口/调试为准。";
}

function bindCommon() {
  $$("[data-route]").forEach(btn => {
    btn.onclick = () => {
      const id = btn.dataset.route;
      if (id) go(id);
    };
  });
  $$("[data-media-tab]").forEach(btn => {
    btn.onclick = () => {
      const wasRoute = state.route;
      state.mediaTab = btn.dataset.mediaTab;
      if (state.route !== "media") state.route = "media";
      location.hash = "#/media";
      render();
      if (wasRoute !== "media") resetPageScroll();
    };
  });
  $$("[data-reboot]").forEach(btn => btn.onclick = rebootDevice);
  $$("[data-sync-time]").forEach(btn => btn.onclick = syncTime);
}

function bindView() {
  if (state.route === "overview") bindOverview();
  if (state.route === "media") bindMedia();
  if (state.route === "display") bindDisplay();
  if (state.route === "network") bindNetwork();
  if (state.route === "system") bindSystem();
}

function bindOverview() {
  $$("[data-app-toggle]").forEach(bindAppToggle);
}

function bindMedia() {
  $$("[data-media-tab]").forEach(btn => {
    btn.onclick = () => { state.mediaTab = btn.dataset.mediaTab; render(); };
  });
  bindDropzones();
  $$("[data-delete]").forEach(btn => btn.onclick = () => confirmDelete(btn.dataset.delete));
  $$("[data-use-image]").forEach(btn => btn.onclick = () => setCurrentImage(btn.dataset.useImage));
  $$("[data-video-fit]").forEach(btn => btn.onclick = () => setVideoFit(btn.dataset.videoFit));
  $("[data-video-audio]")?.addEventListener("change", ev => setVideoAudio(ev.target.checked));
  $$("[data-keytone]").forEach(btn => btn.onclick = () => setKeytone(Number(btn.dataset.keytone)));
  $$("[data-use-tone]").forEach(btn => btn.onclick = () => useTone(btn.dataset.useTone));
  $$("[data-play-tone]").forEach(btn => btn.onclick = () => playTone(btn.dataset.playTone));
  $("[data-retry-ffmpeg]")?.addEventListener("click", () => {
    state.ffmpeg = { status: "idle", message: "" };
    ensureFFmpeg();
    render();
  });
  $("[data-cancel-video]")?.addEventListener("click", cancelVideoTask);
}

function bindDropzones() {
  $$("[data-dropzone]").forEach(zone => {
    const type = zone.dataset.dropzone;
    const input = $(`[data-file-input="${type}"]`);
    if (!input) return;
    zone.onclick = () => input.click();
    zone.onkeydown = ev => {
      if (ev.key === "Enter" || ev.key === " ") {
        ev.preventDefault();
        input.click();
      }
    };
    input.onchange = () => {
      if (input.files?.length) handleFiles(type, [...input.files]);
      input.value = "";
    };
    ["dragenter", "dragover"].forEach(name => zone.addEventListener(name, ev => {
      ev.preventDefault();
      zone.classList.add("dragging");
    }));
    ["dragleave", "drop"].forEach(name => zone.addEventListener(name, ev => {
      ev.preventDefault();
      zone.classList.remove("dragging");
    }));
    zone.addEventListener("drop", ev => {
      if (ev.dataTransfer.files?.length) handleFiles(type, [...ev.dataTransfer.files]);
    });
  });
}

function bindDisplay() {
  $$("[data-theme]").forEach(btn => {
    btn.onclick = () => setTheme(Number(btn.dataset.theme));
  });
  $$("[data-video-fit]").forEach(btn => btn.onclick = () => setVideoFit(btn.dataset.videoFit));
  $("[data-video-audio]")?.addEventListener("change", ev => setVideoAudio(ev.target.checked));
  $$("[data-app-toggle]").forEach(bindAppToggle);
  $("[data-jpg-mode]")?.addEventListener("change", saveJpgSettings);
  $("[data-jpg-file]")?.addEventListener("change", saveJpgSettings);
  $("[data-time-roll]")?.addEventListener("change", saveJpgSettings);
}

function cycleSpaceBreathSpeed() {
  return (state.info.space_breath_speed + 1) % spaceBreathLabels.length;
}

async function setTheme(theme) {
  const wasActiveSpaceTheme = theme === 3 && state.info.theme === 3;
  const speed = wasActiveSpaceTheme ? cycleSpaceBreathSpeed() : state.info.space_breath_speed;
  state.info.theme = theme;
  state.info.space_breath_speed = speed;
  await runAction("theme", () => postForm("/config_theme", {
    theme: String(theme),
    space_breath_speed: String(speed),
  }), wasActiveSpaceTheme ? `呼吸频率：${spaceBreathLabels[speed]}` : "主题已更新");
}

function bindNetwork() {
  $("[data-scan]")?.addEventListener("click", scanNetworks);
  if (state.scan.status === "idle") scanNetworks(false);
  $$("[data-pick-ssid]").forEach(btn => {
    btn.onclick = () => {
      state.selectedSsid = btn.dataset.pickSsid;
      render();
      const input = $("[data-wifi-password]");
      if (input) input.focus();
    };
  });
  $("[data-wifi-ssid]")?.addEventListener("input", ev => { state.selectedSsid = ev.target.value; });
  $("[data-toggle-password]")?.addEventListener("click", () => {
    const input = $("[data-wifi-password]");
    if (!input) return;
    input.type = input.type === "password" ? "text" : "password";
    const button = $("[data-toggle-password]");
    button.innerHTML = input.type === "password" ? I.eye : I.eyeOff;
    button.setAttribute("aria-label", input.type === "password" ? "显示密码" : "隐藏密码");
  });
  $("[data-connect-wifi]")?.addEventListener("click", connectWifi);
}

function bindSystem() {
  $("[data-timezone]")?.addEventListener("change", ev => {
    runAction("timezone", () => postForm("/config_timezone", { timezone: ev.target.value }), "时区已更新");
  });
  const volume = $("[data-volume]");
  volume?.addEventListener("input", ev => {
    state.info.volume = clamp(Number(ev.target.value), 0, 9);
    $(".range-value", ev.target.closest(".range-field"))?.replaceChildren(String(state.info.volume));
  });
  volume?.addEventListener("change", ev => {
    setVolume(ev.target.value);
  });
  $("[data-video-audio]")?.addEventListener("change", ev => setVideoAudio(ev.target.checked));
  $$("[data-app-toggle]").forEach(bindAppToggle);
  bindDropzones();
  $$("[data-play-tone]").forEach(btn => btn.onclick = () => playTone(btn.dataset.playTone));
  $$("[data-delete]").forEach(btn => btn.onclick = () => confirmDelete(btn.dataset.delete));
  $$("[data-cfg]").forEach(input => {
    input.addEventListener("input", ev => {
      const key = ev.target.dataset.cfg;
      if (!key) return;
      if (key === "weather_provider") return;
      const credentialKey = ev.target.dataset.weatherCredential;
      if (credentialKey && state.appCfg?.weatherProvider) {
        state.weatherCredentialDrafts[state.appCfg.weatherProvider] = {
          ...normalizedCredential(state.weatherCredentialDrafts[state.appCfg.weatherProvider] || state.appCfg.weatherProviderCredentials?.[state.appCfg.weatherProvider] || {}),
          [credentialKey]: ev.target.value,
        };
        if (credentialKey === "appcode") {
          state.weatherKeyDrafts[state.appCfg.weatherProvider] = ev.target.value;
        }
      }
      if (key === "weather" && state.appCfg?.weatherProvider) {
        state.weatherKeyDrafts[state.appCfg.weatherProvider] = ev.target.value;
      }
      if (key === "weather_endpoint" && state.appCfg?.weatherProvider) {
        state.weatherEndpointDrafts[state.appCfg.weatherProvider] = ev.target.value;
      }
      state.dirtyAppCfg.add(key);
      state.appCfg = normalizeAppCfg({
        ...state.appCfg,
        weather_provider_endpoints: key === "weather_endpoint" && state.appCfg?.weatherProvider
          ? { ...(state.appCfg.weatherProviderEndpoints || {}), [state.appCfg.weatherProvider]: ev.target.value }
          : state.appCfg?.weatherProviderEndpoints,
        weather_provider_key_values: key === "weather" && state.appCfg?.weatherProvider
          ? { ...(state.appCfg.weatherProviderKeyValues || {}), [state.appCfg.weatherProvider]: ev.target.value }
          : state.appCfg?.weatherProviderKeyValues,
        weather_provider_credentials: credentialKey && state.appCfg?.weatherProvider
          ? { ...(state.appCfg.weatherProviderCredentials || {}), [state.appCfg.weatherProvider]: state.weatherCredentialDrafts[state.appCfg.weatherProvider] }
          : state.appCfg?.weatherProviderCredentials,
        [key]: ev.target.type === "number" ? Number(ev.target.value) : ev.target.value,
      });
    });
  });
  $("[data-weather-provider]")?.addEventListener("change", ev => {
    const previousProvider = state.appCfg?.weatherProvider || "aliyun_72158";
    const previousCredentials = { ...normalizedCredential(state.weatherCredentialDrafts[previousProvider] || state.appCfg?.weatherProviderCredentials?.[previousProvider] || {}) };
    $$("[data-weather-credential]").forEach(input => {
      if (input.dataset.weatherCredential) {
        previousCredentials[input.dataset.weatherCredential] = input.value;
      }
    });
    if (isAliyunProvider(previousProvider)) {
      state.weatherCredentialDrafts[previousProvider] = previousCredentials;
      state.weatherKeyDrafts[previousProvider] = previousCredentials.appcode || "";
    }
    const keyInput = $("[data-cfg=\"weather\"]");
    if (keyInput) {
      state.weatherKeyDrafts[previousProvider] = keyInput.value;
    }
    const endpoint = $("[data-cfg=\"weather_endpoint\"]");
    if (endpoint) {
      state.weatherEndpointDrafts[previousProvider] = endpoint.value;
    }
    const provider = ev.target.value;
    const nextKeyValues = {
      ...defaultWeatherProviderKeyValues(),
      ...(state.appCfg?.weatherProviderKeyValues || {}),
      ...state.weatherKeyDrafts,
    };
    const nextKey = nextKeyValues[provider] || "";
    nextKeyValues[provider] = nextKey;
    const nextCredentials = {
      ...defaultWeatherProviderCredentials(),
      ...(state.appCfg?.weatherProviderCredentials || {}),
      ...state.weatherCredentialDrafts,
    };
    if (isAliyunProvider(provider)) {
      nextCredentials[provider] = normalizedCredential(nextCredentials[provider] || {});
      nextKeyValues[provider] = nextCredentials[provider].appcode || "";
    }
    const nextEndpoints = {
      ...defaultWeatherProviderEndpoints(),
      ...(state.appCfg?.weatherProviderEndpoints || {}),
      ...state.weatherEndpointDrafts,
    };
    const nextEndpoint = nextEndpoints[provider] || defaultWeatherProviderEndpointFor(provider);
    nextEndpoints[provider] = nextEndpoint;
    state.weatherEndpointDrafts[provider] = nextEndpoint;
    if (endpoint) endpoint.value = nextEndpoint;
    state.dirtyAppCfg.add("weather_provider");
    state.dirtyAppCfg.add("weather_endpoint");
    state.appCfg = normalizeAppCfg({
      ...state.appCfg,
      weather_provider_endpoints: nextEndpoints,
      weather_provider_key_values: nextKeyValues,
      weather_provider_credentials: nextCredentials,
      weather_provider: provider,
      weather_endpoint: nextEndpoint,
      weather: isAliyunProvider(provider) ? (nextCredentials[provider]?.appcode || "") : nextKey,
      weather_appcode: nextCredentials[provider]?.appcode || "",
      weather_appkey: nextCredentials[provider]?.appkey || "",
      weather_appsecret: nextCredentials[provider]?.appsecret || "",
      weatherProvider: provider,
      weatherEndpoint: nextEndpoint,
      weatherConfigured: !!state.appCfg?.weatherProviderKeys?.[provider] || providerHasCredential(provider, nextCredentials, nextKeyValues[provider]),
    });
    render();
  });
  $$("[data-language]").forEach(btn => {
    btn.onclick = () => runAction("language", () => postForm("/config_language", { language: btn.dataset.language }), "语言已更新");
  });
  $("[data-detect-city]")?.addEventListener("click", detectWeatherCity);
  $("[data-test-weather]")?.addEventListener("click", testWeatherSource);
  $$("[data-city-pick]").forEach(btn => {
    btn.onclick = () => {
      const input = $("[data-cfg=\"city\"]");
      if (input) {
        input.value = btn.dataset.cityPick;
        state.dirtyAppCfg.add("city");
        state.appCfg = normalizeAppCfg({ ...state.appCfg, city: btn.dataset.cityPick });
        input.focus();
        render();
      }
    };
  });
  $("[data-save-appcfg]")?.addEventListener("click", saveAppConfig);
}

function bindAppToggle(input) {
  input.onchange = () => {
    const appId = input.dataset.appToggle;
    const enabled = input.checked;
    runAction(`app-${appId}`, () => postForm(`/config_app_${appId}`, { enable: String(enabled) }), "应用状态已更新");
  };
}

async function syncTime() {
  await runAction("time", () => postPlain("/time", String(Math.floor(Date.now() / 1000))), "时间已同步");
}

async function rebootDevice() {
  const yes = await confirmDialog({
    title: "重启设备？",
    body: "重启期间屏幕和管理页会短暂不可用。",
    ok: "重启",
    danger: true,
  });
  if (!yes) return;
  try { await postForm("/reboot", {}); } catch (err) {}
  state.online = false;
  toast("重启指令已发送", "info", 5000);
  render();
}

async function setVideoFit(fit) {
  state.info.video_fit = fit === "cover" ? "cover" : "contain";
  await runAction("video-fit", () => postForm("/config_app_gif", {
    enable: String(state.info.gif_enable),
    video_fit: state.info.video_fit,
    video_audio: String(state.info.video_audio),
  }), "视频适配已更新");
}

async function setVideoAudio(enabled) {
  state.info.video_audio = !!enabled;
  await runAction("video-audio", () => postForm("/config_app_gif", {
    enable: String(state.info.gif_enable),
    video_fit: state.info.video_fit,
    video_audio: String(state.info.video_audio),
  }), enabled ? "视频声音已开启" : "视频声音已关闭");
}

async function setVolume(value) {
  const volume = clamp(Number(value), 0, 9);
  state.info.volume = volume;
  await runAction("volume", () => postForm("/config_volume", { volume: String(volume) }), "音量已更新");
}

async function saveJpgSettings() {
  const mode = $("[data-jpg-mode]")?.value || state.info.jpg_mode || "roll";
  const file = $("[data-jpg-file]")?.value || "";
  const roll = Number($("[data-time-roll]")?.value || Math.round(state.info.time_roll / 1000));
  state.info.jpg_mode = mode;
  state.info.jpg_file = file;
  state.info.time_roll = roll * 1000;
  await runAction("jpg", () => postForm("/config_app_jpg", {
    enable: String(state.info.jpg_enable),
    jpg_mode: mode,
    jpg_file: file,
    time_roll: String(roll * 1000),
  }), "显示设置已保存");
}

async function setCurrentImage(name) {
  if (state.info.jpg_mode !== "fixed") {
    const yes = await confirmDialog({
      title: "切换为固定显示？",
      body: `屏幕会立即固定显示 ${name}。`,
      ok: "切换",
    });
    if (!yes) return;
  }
  state.info.jpg_enable = true;
  state.info.jpg_mode = "fixed";
  state.info.jpg_file = name;
  await runAction("current-image", () => postForm("/config_app_jpg", {
    enable: "true",
    jpg_mode: "fixed",
    jpg_file: name,
    time_roll: String(state.info.time_roll || 5000),
  }), "已设为当前显示");
}

async function setKeytone(value) {
  state.info.keytone = value;
  const custom = value === 4 ? (state.info.keytone_file || fileLists().tones[0]?.name || "") : state.info.keytone_file;
  if (value === 4 && custom) state.info.keytone_file = fileName(custom);
  await runAction("keytone", () => postForm("/config_keytone", {
    keytone: String(value),
    keytone_file: value === 4 ? fileName(state.info.keytone_file || "") : fileName(state.info.keytone_file || ""),
  }), "按键音已更新");
}

async function useTone(name) {
  state.info.keytone = 4;
  state.info.keytone_file = name;
  await runAction("tone", () => postForm("/config_keytone", { keytone: "4", keytone_file: name }), "已选用音效");
}

function playTone(name) {
  if (state.audio) {
    state.audio.pause();
    state.audio = null;
    if (state.playing === name) {
      state.playing = "";
      render();
      return;
    }
  }
  const audio = new Audio(fileUrl(name));
  state.audio = audio;
  state.playing = name;
  audio.onended = () => { state.playing = ""; state.audio = null; render(); };
  audio.play().catch(() => toast("无法播放音频", "danger"));
  render();
}

async function saveAppConfig() {
  const cfg = {};
  $$("[data-cfg]").forEach(input => {
    cfg[input.dataset.cfg] = input.type === "number" ? Number(input.value) : input.value;
  });
  cfg.city = (cfg.city || "").trim();
  if (!cfg.city) {
    toast("请填写天气城市", "danger");
    return;
  }
  cfg.weather_provider = cfg.weather_provider || state.appCfg?.weatherProvider || "aliyun_72158";
  const source = weatherSources[cfg.weather_provider] || weatherSources.aliyun_72158;
  cfg.weather_endpoint = (cfg.weather_endpoint || source.endpoint).trim();
  const providerEndpoints = {
    ...defaultWeatherProviderEndpoints(),
    ...(state.appCfg?.weatherProviderEndpoints || {}),
    ...state.weatherEndpointDrafts,
    ...(cfg.weather_provider_endpoints || {}),
    [cfg.weather_provider]: cfg.weather_endpoint,
  };
  weatherProviderIds.forEach(id => {
    const endpoint = (providerEndpoints[id] || weatherSources[id].endpoint || "").trim();
    providerEndpoints[id] = endpoint;
    cfg[`weather_endpoint_${id}`] = endpoint;
  });
  cfg.weather_provider_endpoints = providerEndpoints;
  cfg.weather_lat = (cfg.weather_lat || "").trim();
  cfg.weather_lon = (cfg.weather_lon || "").trim();
  cfg.weather = (cfg.weather || "").trim();
  cfg.weather_appcode = (cfg.weather_appcode || "").trim();
  cfg.weather_appkey = (cfg.weather_appkey || "").trim();
  cfg.weather_appsecret = (cfg.weather_appsecret || "").trim();
  cfg.weather_publickey = (cfg.weather_publickey || state.appCfg?.weatherPublicKeySeniverse || "").trim();
  cfg.weather_publickey_seniverse = cfg.weather_publickey;
  const providerKeys = state.appCfg?.weatherProviderKeys || {};
  const providerCredentials = {
    ...defaultWeatherProviderCredentials(),
    ...(state.appCfg?.weatherProviderCredentials || {}),
    ...state.weatherCredentialDrafts,
  };
  aliyunWeatherProviderIds.forEach(id => {
    providerCredentials[id] = normalizedCredential(providerCredentials[id] || {});
  });
  if (isAliyunProvider(cfg.weather_provider)) {
    providerCredentials[cfg.weather_provider] = {
      ...normalizedCredential(providerCredentials[cfg.weather_provider] || {}),
      appcode: cfg.weather_appcode,
      appkey: cfg.weather_appkey,
      appsecret: cfg.weather_appsecret,
    };
    cfg.weather = cfg.weather_appcode;
  }
  const providerKeyValues = {
    ...defaultWeatherProviderKeyValues(),
    ...(state.appCfg?.weatherProviderKeyValues || {}),
    ...state.weatherKeyDrafts,
    ...(cfg.weather_provider_key_values || {}),
    [cfg.weather_provider]: cfg.weather,
  };
  weatherProviderIds.forEach(id => {
    const key = (providerKeyValues[id] || "").trim();
    providerKeyValues[id] = key;
    cfg[`weather_key_${id}`] = key;
  });
  aliyunWeatherProviderIds.forEach(id => {
    const credential = normalizedCredential(providerCredentials[id] || {});
    providerCredentials[id] = credential;
    providerKeyValues[id] = credential.appcode;
    cfg[`weather_key_${id}`] = credential.appcode;
    cfg[`weather_appcode_${id}`] = credential.appcode;
    cfg[`weather_appkey_${id}`] = credential.appkey;
    cfg[`weather_appsecret_${id}`] = credential.appsecret;
  });
  cfg.weather_provider_key_values = providerKeyValues;
  cfg.weather_provider_credentials = providerCredentials;
  const hasSavedKey = !!providerKeys[cfg.weather_provider];
  const nextProviderKeys = { ...providerKeys };
  weatherProviderIds.forEach(id => {
    if (providerHasCredential(id, providerCredentials, providerKeyValues[id])) nextProviderKeys[id] = true;
  });
  if (!providerHasCredential(cfg.weather_provider, providerCredentials, providerKeyValues[cfg.weather_provider]) && !hasSavedKey) {
    toast(isAliyunProvider(cfg.weather_provider) ? "这个阿里云天气源需要 AppCode 或 AppKey/AppSecret" : "这个天气源需要 API Key，请先填写", "danger");
    return;
  }
  const nextAppCfg = normalizeAppCfg({
    ...state.appCfg,
    ...cfg,
    weather_provider_endpoints: providerEndpoints,
    weather_provider_key_values: providerKeyValues,
    weather_provider_credentials: providerCredentials,
    weather_provider_keys: nextProviderKeys,
  });
  const saved = await runAction("appcfg", () => postPlain("/config.json", JSON.stringify(cfg)), "应用参数已保存");
  if (saved) {
    state.appCfg = nextAppCfg;
    state.weatherKeyDrafts = {};
    state.weatherCredentialDrafts = {};
    state.weatherEndpointDrafts = {};
    state.dirtyAppCfg.clear();
    render();
  }
}

function collectWeatherDraft() {
  const provider = $("[data-weather-provider]")?.value || state.appCfg?.weatherProvider || "aliyun_72158";
  const source = weatherSources[provider] || weatherSources.aliyun_72158;
  const credential = isAliyunProvider(provider)
    ? {
        ...normalizedCredential(state.appCfg?.weatherProviderCredentials?.[provider] || {}),
        ...normalizedCredential(state.weatherCredentialDrafts?.[provider] || {}),
        appcode: ($("[data-weather-credential=\"appcode\"]")?.value || state.weatherCredentialDrafts?.[provider]?.appcode || state.appCfg?.weatherProviderCredentials?.[provider]?.appcode || "").trim(),
        appkey: ($("[data-weather-credential=\"appkey\"]")?.value || state.weatherCredentialDrafts?.[provider]?.appkey || state.appCfg?.weatherProviderCredentials?.[provider]?.appkey || "").trim(),
        appsecret: ($("[data-weather-credential=\"appsecret\"]")?.value || state.weatherCredentialDrafts?.[provider]?.appsecret || state.appCfg?.weatherProviderCredentials?.[provider]?.appsecret || "").trim(),
      }
    : { appcode: "", appkey: "", appsecret: "" };
  const key = isAliyunProvider(provider)
    ? credential.appcode
    : ($("[data-cfg=\"weather\"]")?.value || state.appCfg?.weatherProviderKeyValues?.[provider] || "").trim();
  return {
    weather_provider: provider,
    weather_endpoint: ($("[data-cfg=\"weather_endpoint\"]")?.value || state.appCfg?.weatherProviderEndpoints?.[provider] || source.endpoint || "").trim(),
    weather: key,
    weather_appcode: credential.appcode,
    weather_appkey: credential.appkey,
    weather_appsecret: credential.appsecret,
    weather_publickey: provider === "seniverse" ? ($("[data-weather-seniverse=\"publickey\"]")?.value || state.appCfg?.weatherPublicKeySeniverse || "").trim() : "",
    city: ($("[data-cfg=\"city\"]")?.value || state.appCfg?.city || "").trim(),
    weather_lat: ($("[data-cfg=\"weather_lat\"]")?.value || state.appCfg?.weatherLat || "").trim(),
    weather_lon: ($("[data-cfg=\"weather_lon\"]")?.value || state.appCfg?.weatherLon || "").trim(),
  };
}

async function testWeatherSource() {
  const cfg = collectWeatherDraft();
  if (!cfg.city) {
    state.weatherTest = { status: "error", message: "请先填写城市再测试" };
    render();
    return;
  }
  if (isAliyunProvider(cfg.weather_provider) && !cfg.weather_appcode && !(cfg.weather_appkey && cfg.weather_appsecret)) {
    state.weatherTest = { status: "error", message: "请先填写阿里云 AppCode，或同时填写 AppKey 和 AppSecret" };
    render();
    return;
  }
  if (cfg.weather_provider === "seniverse" && (!cfg.weather_publickey || !cfg.weather)) {
    state.weatherTest = { status: "error", message: "请填写心知天气公钥和私钥" };
    render();
    return;
  }
  if (!isAliyunProvider(cfg.weather_provider) && cfg.weather_provider !== "seniverse" && !cfg.weather) {
    state.weatherTest = { status: "error", message: "请先填写当前天气源的 API Key" };
    render();
    return;
  }
  setPending("weather-test", true);
  state.weatherTest = { status: "testing", message: "正在让设备测试天气接口..." };
  render();
  try {
    const res = await fetch("/weather_test", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(cfg),
    });
    const data = await res.json().catch(() => ({}));
    state.weatherTest = {
      status: data.ok ? "ok" : "error",
      message: data.message || (res.ok ? "测试完成，但设备没有返回详情" : `测试失败：HTTP ${res.status}`),
    };
  } catch (err) {
    state.weatherTest = { status: "error", message: "测试失败：无法连接设备天气测试接口" };
  } finally {
    setPending("weather-test", false);
    render();
  }
}

function setWeatherDetect(status, message = "") {
  state.weatherDetect = { status, message };
  render();
}

function pickCityFromGeo(data) {
  const city = data?.city || data?.locality || data?.principalSubdivision || data?.region || data?.countryName;
  if (!city) return "";
  return String(city).replace(/市$/, "").trim();
}

async function cityFromCoordinates(latitude, longitude) {
  const url = `https://api.bigdatacloud.net/data/reverse-geocode-client?latitude=${encodeURIComponent(latitude)}&longitude=${encodeURIComponent(longitude)}&localityLanguage=zh`;
  const res = await fetch(url, { cache: "no-store" });
  if (!res.ok) throw new Error(`geo ${res.status}`);
  return { city: pickCityFromGeo(await res.json()), lat: latitude, lon: longitude };
}

async function cityFromIp() {
  const res = await fetch("https://api.bigdatacloud.net/data/reverse-geocode-client?localityLanguage=zh", { cache: "no-store" });
  if (!res.ok) throw new Error(`ip ${res.status}`);
  const data = await res.json();
  return { city: pickCityFromGeo(data), lat: data.latitude, lon: data.longitude };
}

function browserPosition(timeout = 9000) {
  return new Promise((resolve, reject) => {
    if (!navigator.geolocation) {
      reject(new Error("geolocation unavailable"));
      return;
    }
    navigator.geolocation.getCurrentPosition(resolve, reject, {
      enableHighAccuracy: false,
      timeout,
      maximumAge: 30 * 60 * 1000,
    });
  });
}

async function detectWeatherCity() {
  setWeatherDetect("detecting", "正在识别当前位置");
  let detected = { city: "", lat: "", lon: "" };
  try {
    const pos = await browserPosition();
    detected = await cityFromCoordinates(pos.coords.latitude, pos.coords.longitude);
  } catch (err) {
    try {
      setWeatherDetect("detecting", "定位未授权，正在尝试按网络识别");
      detected = await cityFromIp();
    } catch (fallbackErr) {
      setWeatherDetect("idle", "自动识别失败，请手动输入城市");
      toast("自动识别失败，请手动输入城市", "danger");
      return;
    }
  }

  const city = detected.city || "";
  if (!city) {
    setWeatherDetect("idle", "没有识别到城市，请手动输入");
    toast("没有识别到城市", "danger");
    return;
  }

  const input = $("[data-cfg=\"city\"]");
  if (input) input.value = city;
  const lat = $("[data-cfg=\"weather_lat\"]");
  const lon = $("[data-cfg=\"weather_lon\"]");
  const latValue = Number(detected.lat);
  const lonValue = Number(detected.lon);
  if (lat && Number.isFinite(latValue)) lat.value = latValue.toFixed(4);
  if (lon && Number.isFinite(lonValue)) lon.value = lonValue.toFixed(4);
  state.dirtyAppCfg.add("city");
  state.dirtyAppCfg.add("weather_lat");
  state.dirtyAppCfg.add("weather_lon");
  state.appCfg = normalizeAppCfg({
    ...state.appCfg,
    city,
    weatherLat: lat?.value || state.appCfg?.weatherLat || "",
    weatherLon: lon?.value || state.appCfg?.weatherLon || "",
  });
  setWeatherDetect("idle", `已识别为 ${city}，保存后同步到屏幕`);
  toast(`已识别为 ${city}`);
}

async function scanNetworks(doRender = true) {
  state.scan = { status: "scanning", networks: [] };
  if (doRender) render();
  try {
    const res = await getJSON("/scan_networks");
    const seen = new Set();
    const networks = (res.networks || [])
      .filter(n => n.ssid && !seen.has(n.ssid) && seen.add(n.ssid))
      .sort((a, b) => Number(b.rssi) - Number(a.rssi));
    state.scan = { status: "done", networks };
  } catch (err) {
    state.scan = { status: "done", networks: [] };
    toast("扫描失败", "danger");
  }
  render();
}

async function connectWifi() {
  const ssid = ($("[data-wifi-ssid]")?.value || "").trim();
  const password = $("[data-wifi-password]")?.value || "";
  if (!ssid) {
    toast("请先输入 SSID", "danger");
    return;
  }
  const yes = await confirmDialog({
    title: `连接到 ${ssid}？`,
    body: "设备会尝试加入新网络，成功后可能需要用屏幕上的新 IP 访问管理页。",
    ok: "连接",
  });
  if (!yes) return;
  try {
    await postForm("/config_wifi", { ssid, password });
  } catch (err) {
    // Switching Wi-Fi can interrupt the HTTP request. Treat it as sent.
  }
  modal(`<h2>配置已发送</h2><p class="modal-copy">请等待屏幕连接新 Wi-Fi。如果 IP 变化，用屏幕显示的新地址重新打开管理页。</p><div class="button-row"><button class="btn primary" data-close-modal>知道了</button></div>`);
  $("[data-close-modal]")?.addEventListener("click", () => modals.lastElementChild?.remove());
}

async function handleFiles(type, files) {
  const file = files[0];
  if (!file) return;
  if (type === "image") return openCropper(file);
  if (type === "video") return startVideoUpload(file);
  if (type === "tone") return uploadTone(file);}

function safeName(original, ext) {
  const names = new Set((state.list?.data || []).map(f => fileName(f.name)));
  let base = fileName(original).replace(/\.[^.]+$/, "").replace(/[^\w-]+/g, "_").replace(/^_+|_+$/g, "").slice(0, 24);
  if (!base) base = "screen_file";
  let name = `${base}${ext}`;
  let i = 1;
  while (names.has(name)) name = `${base}_${i++}${ext}`;
  return name;
}

function openCropper(file) {
  if (!/^image\//.test(file.type)) {
    toast("请选择图片文件", "danger");
    return;
  }
  const url = URL.createObjectURL(file);
  const img = new Image();
  img.onload = () => {
    URL.revokeObjectURL(url);
    const m = modal(`
      <h2>裁剪图片</h2>
      <div class="crop-shell"><canvas width="640" height="480" data-crop-canvas></canvas></div>
      <label class="field"><span>缩放</span><input type="range" min="0" max="100" value="0" data-crop-zoom></label>
      <div data-crop-progress></div>
      <div class="button-row"><button class="btn ghost" data-cancel>取消</button><button class="btn primary" data-upload>${I.upload}<span>上传</span></button></div>
    `, { wide: true });
    const canvas = $("[data-crop-canvas]", m.card);
    const ctx = canvas.getContext("2d");
    const W = canvas.width;
    const H = canvas.height;
    const cover = Math.max(W / img.width, H / img.height);
    let zoom = 1;
    let ox = 0;
    let oy = 0;
    let drag = null;
    const draw = () => {
      const scale = cover * zoom;
      const dw = img.width * scale;
      const dh = img.height * scale;
      const maxX = Math.max(0, (dw - W) / 2);
      const maxY = Math.max(0, (dh - H) / 2);
      ox = clamp(ox, -maxX, maxX);
      oy = clamp(oy, -maxY, maxY);
      ctx.fillStyle = "#05070d";
      ctx.fillRect(0, 0, W, H);
      ctx.drawImage(img, (W - dw) / 2 + ox, (H - dh) / 2 + oy, dw, dh);
    };
    draw();
    canvas.onpointerdown = ev => {
      drag = { x: ev.clientX, y: ev.clientY };
      canvas.setPointerCapture(ev.pointerId);
    };
    canvas.onpointermove = ev => {
      if (!drag) return;
      const scale = W / canvas.clientWidth;
      ox += (ev.clientX - drag.x) * scale;
      oy += (ev.clientY - drag.y) * scale;
      drag = { x: ev.clientX, y: ev.clientY };
      draw();
    };
    canvas.onpointerup = canvas.onpointercancel = () => { drag = null; };
    $("[data-crop-zoom]", m.card).oninput = ev => {
      zoom = 1 + Number(ev.target.value) / 100 * 3;
      draw();
    };
    $("[data-cancel]", m.card).onclick = m.close;
    $("[data-upload]", m.card).onclick = () => {
      const out = document.createElement("canvas");
      out.width = SCREEN_W;
      out.height = SCREEN_H;
      out.getContext("2d").drawImage(canvas, 0, 0, W, H, 0, 0, SCREEN_W, SCREEN_H);
      out.toBlob(async blob => {
        if (!blob) return toast("图片处理失败", "danger");
        if (!hasSpace(blob.size)) return;
        const name = safeName(file.name, ".jpg");
        await uploadBlobWithProgress(blob, name, $("[data-crop-progress]", m.card));
        m.close();
        await refreshAll(true);
        toast("图片已上传");
        render();
      }, "image/jpeg", 0.86);
    };
  };
  img.onerror = () => {
    URL.revokeObjectURL(url);
    toast("无法读取图片", "danger");
  };
  img.src = url;
}

function hasSpace(size) {
  if (state.list && state.list.size + size > state.list.total) {
    toast("存储空间不足，请先删除文件", "danger");
    return false;
  }
  return true;
}

async function uploadBlobWithProgress(blob, name, host) {
  if (host) host.innerHTML = progressLine("上传", 0, name);
  await uploadFile(blob, name, pct => {
    if (host) host.innerHTML = progressLine("上传", pct * 100, name);
  });
}

async function uploadTone(file) {
  await uploadToneFile(file);
}

async function uploadToneFile(file, okMessage = "音效已上传") {
  const ext = extOf(file.name);
  if (![".mp3", ".wav"].includes(ext)) {
    toast("请选择 MP3 或 WAV", "danger");
    return "";
  }
  if (!hasSpace(file.size)) return "";
  const name = safeName(file.name, ext);
  try {
    await uploadFile(file, name);
    await refreshAll(true);
    toast(okMessage);
    render();
    return name;
  } catch (err) {
    toast("上传失败", "danger");
    return "";
  }
}

async function ensureFFmpeg() {
  if (state.ffmpeg.status === "ready") return ffmpegInstance;
  if (state.ffmpeg.status === "loading") return null;
  state.ffmpeg = { status: "loading", message: "" };
  render();
  try {
    const mod = await import("/ffmpeg.js");
    ffmpegInstance = new mod.F();
    const base = "https://unpkg.com/@ffmpeg/core@0.12.6/dist/esm";
    const [coreURL, wasmURL] = await Promise.all([
      toBlobURL(base + "/ffmpeg-core.js", "text/javascript"),
      toBlobURL(base + "/ffmpeg-core.wasm", "application/wasm"),
    ]);
    await ffmpegInstance.load({ coreURL, wasmURL });
    state.ffmpeg = { status: "ready", message: "" };
  } catch (err) {
    ffmpegInstance = null;
    state.ffmpeg = { status: "error", message: String(err) };
  }
  render();
  return ffmpegInstance;
}

async function toBlobURL(url, mime) {
  const res = await fetch(url);
  if (!res.ok) throw new Error(`fetch ${url}`);
  return URL.createObjectURL(new Blob([await res.arrayBuffer()], { type: mime }));
}

async function startVideoUpload(file) {
  const ext = extOf(file.name);
  if (ext === ".mpeg" || ext === ".mpg") {
    if (!hasSpace(file.size)) return;
    const name = safeName(file.name, ".mpeg");
    try {
      await uploadFile(file, name);
      await refreshAll(true);
      toast("视频已上传");
      render();
    } catch (err) {
      toast("上传失败", "danger");
    }
    return;
  }

  if (![".mp4", ".gif"].includes(ext)) {
    toast("请选择 MP4、GIF 或 MPEG", "danger");
    return;
  }

  state.videoTask = { name: file.name, stage: "transcode", transcode: 0, upload: 0, error: "" };
  render();
  const ff = await ensureFFmpeg();
  if (!ff) {
    state.videoTask = null;
    render();
    return;
  }

  const input = ext === ".gif" ? "input.gif" : "input.mp4";
  const output = "output.mpeg";
  const fit = state.info?.video_fit === "cover" ? "cover" : "contain";
  const audioWanted = !!state.info?.video_audio;
  const vf = fit === "cover"
    ? `scale=${VIDEO_TARGET_W}:${VIDEO_TARGET_H}:force_original_aspect_ratio=increase:flags=fast_bilinear,crop=${VIDEO_TARGET_W}:${VIDEO_TARGET_H},fps=${VIDEO_TARGET_FPS}`
    : `scale=${VIDEO_TARGET_W}:${VIDEO_TARGET_H}:force_original_aspect_ratio=decrease:flags=fast_bilinear,pad=${VIDEO_TARGET_W}:${VIDEO_TARGET_H}:(ow-iw)/2:(oh-ih)/2:black,fps=${VIDEO_TARGET_FPS}`;
  const onProgress = ({ progress }) => {
    if (!state.videoTask) return;
    state.videoTask.transcode = clamp(progress * 100, 0, 100);
    updateVideoTask();
  };

  try {
    ff.on("progress", onProgress);
    await ff.writeFile(input, new Uint8Array(await file.arrayBuffer()));
    const args = [
      "-i", input,
      "-map", "0:v:0",
      "-vf", vf,
      "-c:v", "mpeg1video",
      "-b:v", VIDEO_BITRATE,
      "-maxrate", VIDEO_MAXRATE,
      "-bf", "0",
    ];
    audioWanted
      ? args.push(
          "-map", "0:a?",
          "-c:a", "mp2",
          "-b:a", VIDEO_AUDIO_BITRATE,
          "-ar", VIDEO_AUDIO_RATE,
          "-ac", VIDEO_AUDIO_CHANNELS,
          "-shortest",
        )
      : args.push("-an");
    args.push("-f", "mpeg", output);
    const rc = await ff.exec(args);
    if (rc !== 0) throw new Error(`ffmpeg rc=${rc}`);
    const data = await ff.readFile(output);
    try { await ff.deleteFile(input); await ff.deleteFile(output); } catch (err) {}
    const blob = new Blob([data.buffer ?? data], { type: "video/mpeg" });
    if (!hasSpace(blob.size)) {
      state.videoTask.error = "存储空间不足";
      updateVideoTask();
      return;
    }
    state.videoTask.stage = "upload";
    state.videoTask.transcode = 100;
    updateVideoTask();
    const name = safeName(file.name, ".mpeg");
    await uploadFile(blob, name, pct => {
      if (!state.videoTask) return;
      state.videoTask.upload = pct * 100;
      updateVideoTask();
    });
    state.videoTask = null;
    await refreshAll(true);
    toast("视频已转码并上传");
    render();
  } catch (err) {
    if (state.videoTask) {
      state.videoTask.error = "转码失败，请换一个文件重试";
      updateVideoTask();
    }
  } finally {
    try { ff.off && ff.off("progress", onProgress); } catch (err) {}
  }
}

function updateVideoTask() {
  const holder = $(".task-card");
  if (!state.videoTask || !holder) {
    render();
    return;
  }
  holder.outerHTML = videoTaskHTML(state.videoTask);
  $("[data-cancel-video]")?.addEventListener("click", cancelVideoTask);
}

function cancelVideoTask() {
  if (!state.videoTask) return;
  if (state.videoTask.stage === "transcode" && ffmpegInstance) {
    try { ffmpegInstance.terminate(); } catch (err) {}
    ffmpegInstance = null;
    state.ffmpeg = { status: "idle", message: "" };
  }
  state.videoTask = null;
  toast("已取消", "info");
  render();
}

async function confirmDelete(name) {
  const yes = await confirmDialog({
    title: `删除 ${name}？`,
    body: "删除后无法撤销。",
    ok: "删除",
    danger: true,
  });
  if (!yes) return;
  try {
    await deleteFile(name);
    if (state.info?.jpg_file === name) state.info.jpg_file = "";
    if (state.info?.keytone_file === name) {
      state.info.keytone = 0;
      state.info.keytone_file = "";
      try { await postForm("/config_keytone", { keytone: "0", keytone_file: "" }); } catch (err) {}
    }
    await refreshAll(true);
    toast("已删除");
    render();
  } catch (err) {
    toast("删除失败", "danger");
  }
}

function portalView() {
  return `
    <main class="portal-page">
      <section class="portal-card glass-shell">
        <div class="brand-lockup center">${I.logo}<div><b>SKYLOONG 4.0</b><small>连接到 Wi-Fi</small></div></div>
        <p class="portal-copy">选择 2.4GHz 网络，设备连接成功后会在屏幕上显示新的 IP。</p>
        ${viewNetwork().replace("网络", "配网").replace("扫描、保存并切换 Wi-Fi", "选择屏幕要加入的无线网络")}
      </section>
    </main>
  `;
}

function bindPortal() {
  bindNetwork();
}

async function boot() {
  if ("scrollRestoration" in history) history.scrollRestoration = "manual";
  state.route = routeFromHash();
  window.addEventListener("hashchange", () => {
    const previousRoute = state.route;
    state.route = routeFromHash();
    render();
    if (previousRoute !== state.route) resetPageScroll();
  });
  await refreshAll(true);
  render();
  resetPageScroll();
  pollTimer = window.setInterval(async () => {
    await refreshAll(true);
    render();
  }, 7000);
}

boot();
