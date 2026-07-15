import assert from "node:assert/strict";
import fs from "node:fs";
import path from "node:path";
import vm from "node:vm";
import { fileURLToPath } from "node:url";

const repoRoot = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "..");
const webSources = ["web_new/index.js", "web/index.js"];

function createHarness(relativePath, files) {
  const sourcePath = path.join(repoRoot, relativePath);
  const source = fs.readFileSync(sourcePath, "utf8");
  const instrumented = source.replace(
    /boot\(\);\s*$/,
    "globalThis.__navigationTest = { state, render };",
  );
  assert.notEqual(instrumented, source, `${relativePath} must end by calling boot()`);

  const elements = {
    app: { innerHTML: "" },
    toasts: { innerHTML: "" },
    modals: { innerHTML: "", lastElementChild: null },
  };
  const document = {
    body: { toggleAttribute() {} },
    activeElement: null,
    getElementById(id) {
      return elements[id] ?? null;
    },
    querySelector() {
      return null;
    },
    querySelectorAll() {
      return [];
    },
  };
  const location = { pathname: "/", hash: "#/media" };
  const window = {
    location,
    addEventListener() {},
    scrollTo() {},
    setInterval() {},
  };
  const context = vm.createContext({
    console,
    document,
    history: {},
    location,
    requestAnimationFrame(callback) {
      callback();
    },
    window,
  });

  vm.runInContext(instrumented, context, { filename: sourcePath });
  const { state, render } = context.__navigationTest;
  state.route = "media";
  state.mediaTab = "tones";
  state.info = {
    mode: "STA",
    ssid: "navigation-test",
    ip: "127.0.0.1",
    keytone: 1,
    keytone_file: "",
  };
  state.list = { size: 0, total: 1024, data: files };

  render();
  return elements.app.innerHTML;
}

const cases = [
  {
    name: "renders an empty keytone library",
    files: [],
    verify(html) {
      assert.match(html, /媒体库/);
      assert.match(html, /0 个文件/);
      assert.match(html, /还没有自定义音效/);
    },
  },
  {
    name: "renders a WAV keytone",
    files: [{ type: "file", name: "tap.wav", size: 42 }],
    verify(html) {
      assert.match(html, /1 个文件/);
      assert.match(html, /tap\.wav/);
      assert.match(html, /data-use-tone="tap\.wav"/);
    },
  },
];

const failures = [];
for (const relativePath of webSources) {
  for (const testCase of cases) {
    try {
      const html = createHarness(relativePath, testCase.files);
      testCase.verify(html);
      console.log(`PASS ${relativePath}: ${testCase.name}`);
    } catch (error) {
      failures.push({ relativePath, testCase: testCase.name, error });
      console.error(`FAIL ${relativePath}: ${testCase.name}`);
      console.error(`  ${error.name}: ${error.message}`);
    }
  }
}

if (failures.length > 0) {
  console.error(`\n${failures.length} web navigation regression test(s) failed.`);
  process.exitCode = 1;
}
