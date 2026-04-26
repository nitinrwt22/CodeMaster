import { useState, useRef, useCallback, useEffect } from 'react';
import { analyzeCode } from '../services/api';
import {
  UploadCloud, FileType, CheckCircle, AlertCircle,
  Code2, Play, X, Zap, FileCode
} from 'lucide-react';
import './Analyze.css';

// ── C keyword list for very basic token colouring ──────────────────────────
const C_KEYWORDS = new Set([
  'auto','break','case','char','const','continue','default','do','double',
  'else','enum','extern','float','for','goto','if','inline','int','long',
  'register','restrict','return','short','signed','sizeof','static','struct',
  'switch','typedef','union','unsigned','void','volatile','while','NULL',
  'true','false','include','define','ifdef','ifndef','endif','pragma',
]);

/**
 * Tokenise a line of C code into colored <span> elements.
 * Very lightweight — handles keywords, strings, chars, comments, numbers.
 */
function tokeniseLine(line) {
  // Single-pass regex tokenizer
  const tokens = [];
  const re = /(\/\/.*$)|(\/\*[\s\S]*?\*\/)|(#\s*\w+)|("(?:[^"\\]|\\.)*"|'(?:[^'\\]|\\.)*')|(\b\d+(?:\.\d+)?\b)|(\b[A-Za-z_]\w*\b)|([^A-Za-z0-9_\s"'])/g;
  let m;
  let lastIdx = 0;
  while ((m = re.exec(line)) !== null) {
    if (m.index > lastIdx) tokens.push({ cls: '', text: line.slice(lastIdx, m.index) });
    const [full, comment, mlComment, preproc, string, number, ident, other] = m;
    if (comment || mlComment)          tokens.push({ cls: 'tok-comment', text: full });
    else if (preproc)                  tokens.push({ cls: 'tok-preproc',  text: full });
    else if (string)                   tokens.push({ cls: 'tok-string',   text: full });
    else if (number)                   tokens.push({ cls: 'tok-number',   text: full });
    else if (ident && C_KEYWORDS.has(ident)) tokens.push({ cls: 'tok-keyword', text: full });
    else if (ident)                    tokens.push({ cls: 'tok-ident',    text: full });
    else                               tokens.push({ cls: '',             text: full });
    lastIdx = m.index + full.length;
  }
  if (lastIdx < line.length) tokens.push({ cls: '', text: line.slice(lastIdx) });
  return tokens;
}

// ── Code Editor with line numbers + syntax colours ─────────────────────────
function CodeEditor({ value, onChange, placeholder }) {
  const textareaRef = useRef(null);
  const highlightRef = useRef(null);
  const gutterRef    = useRef(null);

  // Sync scroll between textarea and highlight layer
  const syncScroll = useCallback(() => {
    if (!textareaRef.current || !highlightRef.current || !gutterRef.current) return;
    const { scrollTop, scrollLeft } = textareaRef.current;
    highlightRef.current.scrollTop  = scrollTop;
    highlightRef.current.scrollLeft = scrollLeft;
    gutterRef.current.scrollTop     = scrollTop;
  }, []);

  const lines = value ? value.split('\n') : [''];

  // Tab key support
  const handleKeyDown = (e) => {
    if (e.key === 'Tab') {
      e.preventDefault();
      const ta = e.target;
      const start = ta.selectionStart;
      const end   = ta.selectionEnd;
      const newVal = ta.value.substring(0, start) + '    ' + ta.value.substring(end);
      onChange({ target: { value: newVal } });
      requestAnimationFrame(() => {
        ta.selectionStart = ta.selectionEnd = start + 4;
      });
    }
  };

  return (
    <div className="ce-wrap">
      {/* Line numbers */}
      <div className="ce-gutter" ref={gutterRef}>
        {lines.map((_, i) => (
          <div key={i} className="ce-gutter-line">{i + 1}</div>
        ))}
      </div>

      {/* Highlight overlay */}
      <div className="ce-highlight-wrap">
        <div className="ce-highlight" ref={highlightRef} aria-hidden="true">
          {lines.map((line, i) => (
            <div key={i} className="ce-hl-line">
              {tokeniseLine(line).map((tok, j) =>
                tok.cls
                  ? <span key={j} className={tok.cls}>{tok.text}</span>
                  : tok.text
              )}
              {'\n'}
            </div>
          ))}
        </div>

        {/* Transparent textarea on top */}
        <textarea
          ref={textareaRef}
          className="ce-textarea"
          value={value}
          onChange={onChange}
          onScroll={syncScroll}
          onKeyDown={handleKeyDown}
          placeholder={placeholder}
          spellCheck={false}
          autoCapitalize="off"
          autoCorrect="off"
        />
      </div>
    </div>
  );
}

// ── Main Analyze Page ──────────────────────────────────────────────────────
export default function Analyze() {
  const [tab, setTab]           = useState('upload');
  const [isDragging, setIsDragging] = useState(false);
  const [status, setStatus]     = useState('idle'); // idle | loading | success | error
  const [code, setCode]         = useState('');
  const [fileName, setFileName] = useState('');
  const [lineCount, setLineCount] = useState(1);
  const fileRef                 = useRef(null);

  useEffect(() => {
    setLineCount(code ? code.split('\n').length : 1);
  }, [code]);

  const runAnalysis = useCallback(async (source) => {
    setStatus('loading');
    try {
      await analyzeCode(source);
      setStatus('success');
    } catch {
      setStatus('error');
    }
  }, []);

  const handleFile = async (file) => {
    if (!file) return;
    setFileName(file.name);
    const text = await file.text();
    setCode(text);
    setTab('paste'); // Switch to editor tab so they see the loaded code
    await runAnalysis(text);
  };

  const handleRun = () => {
    if (!code.trim()) return;
    runAnalysis(code);
  };

  const clearAll = () => {
    setCode('');
    setFileName('');
    setStatus('idle');
  };

  const statusMeta = {
    loading: { cls: 'an-status--loading', icon: <FileType size={15} className="an-spin" />, text: 'Analyzing your code…' },
    success: { cls: 'an-status--success', icon: <CheckCircle size={15} />, text: `Analysis complete! Dashboard metrics have been updated.` },
    error:   { cls: 'an-status--error',   icon: <AlertCircle size={15} />, text: 'Analysis failed. Check your C code and try again.' },
  };

  return (
    <div className="an-page">
      {/* Page Header */}
      <div className="an-header">
        <div className="an-header-left">
          <span className="an-eyebrow">Code Input</span>
          <h1 className="an-title">Analyze <span>Source Code</span></h1>
          <p className="an-subtitle">Upload a .c/.cpp file or paste code directly. Instant static analysis powered by CodeMaster.</p>
        </div>
        <div className="an-header-badges">
          <span className="an-badge"><Zap size={11} /> CodeMaster v1.0</span>
          <span className="an-badge an-badge--dim"><FileCode size={11} /> C / C++ / H</span>
        </div>
      </div>

      {/* Status Banner */}
      {status !== 'idle' && (
        <div className={`an-status-bar ${statusMeta[status].cls}`}>
          {statusMeta[status].icon}
          <span>{statusMeta[status].text}</span>
          {status !== 'loading' && (
            <button className="an-status-dismiss" onClick={() => setStatus('idle')}>
              <X size={13} />
            </button>
          )}
        </div>
      )}

      {/* Tab Bar */}
      <div className="an-tabs">
        <button
          id="analyze-tab-upload"
          className={`an-tab ${tab === 'upload' ? 'active' : ''}`}
          onClick={() => setTab('upload')}
        >
          <UploadCloud size={14} /> Upload File
        </button>
        <button
          id="analyze-tab-paste"
          className={`an-tab ${tab === 'paste' ? 'active' : ''}`}
          onClick={() => setTab('paste')}
        >
          <Code2 size={14} /> Code Editor
        </button>
      </div>

      {/* ── Upload Tab ── */}
      {tab === 'upload' && (
        <div
          className={`an-dropzone ${isDragging ? 'dragging' : ''}`}
          onClick={() => fileRef.current?.click()}
          onDragOver={(e) => { e.preventDefault(); setIsDragging(true); }}
          onDragLeave={() => setIsDragging(false)}
          onDrop={(e) => {
            e.preventDefault();
            setIsDragging(false);
            handleFile(e.dataTransfer.files[0]);
          }}
        >
          <input
            type="file" accept=".c,.cpp,.h" ref={fileRef}
            style={{ display: 'none' }}
            onChange={(e) => handleFile(e.target.files[0])}
          />
          <div className="an-dropzone-icon">
            {status === 'loading' ? <FileType size={36} className="an-spin" />
             : status === 'success' ? <CheckCircle size={36} style={{ color: 'var(--color-secondary)' }} />
             : <UploadCloud size={36} />}
          </div>
          <div className="an-dropzone-content">
            {status === 'idle' && (
              <>
                <h3>Drop your file here</h3>
                <p>Supports <code>.c</code> <code>.cpp</code> <code>.h</code> — or click to browse</p>
              </>
            )}
            {status === 'loading' && <><h3>Analyzing {fileName || 'file'}…</h3><p>Running CodeMaster pipeline</p></>}
            {status === 'success' && <><h3>{fileName} analyzed!</h3><p>Switch to Code Editor tab to view the loaded source</p></>}
            {status === 'error'   && <><h3>Analysis Failed</h3><p>Could not process the file. Try again.</p></>}
          </div>
          <div className="an-dropzone-hint">
            <span>MAX FILE SIZE</span><strong>10 MB</strong>
          </div>
        </div>
      )}

      {/* ── Code Editor Tab ── */}
      {tab === 'paste' && (
        <div className="an-editor-panel">
          {/* Editor toolbar */}
          <div className="an-editor-bar">
            <div className="an-editor-bar-left">
              <div className="an-traffic-lights">
                <span className="an-tl an-tl--red" />
                <span className="an-tl an-tl--yellow" />
                <span className="an-tl an-tl--green" />
              </div>
              <span className="an-editor-filename">
                {fileName || 'untitled.c'}
              </span>
            </div>
            <div className="an-editor-bar-right">
              <span className="an-editor-meta">{lineCount} lines</span>
              {code && (
                <button className="an-clear-btn" onClick={clearAll}>
                  <X size={12} /> Clear
                </button>
              )}
              <button
                id="analyze-run-btn"
                className="an-run-btn"
                onClick={handleRun}
                disabled={!code.trim() || status === 'loading'}
              >
                {status === 'loading'
                  ? <><FileType size={13} className="an-spin" /> Analyzing…</>
                  : <><Play size={13} fill="currentColor" /> Run Analysis</>}
              </button>
            </div>
          </div>

          {/* Code Editor */}
          <CodeEditor
            value={code}
            onChange={(e) => setCode(e.target.value)}
            placeholder={`// Paste or type your C/C++ code here…\n#include <stdio.h>\n\nint main() {\n    printf("Hello, CodeMaster!\\n");\n    return 0;\n}`}
          />

          {/* Footer bar */}
          <div className="an-editor-footer">
            <span className="an-footer-meta">C / C++</span>
            <span className="an-footer-meta">UTF-8</span>
            <span className="an-footer-meta">Tab: 4 spaces</span>
            <span className="an-footer-meta" style={{ marginLeft: 'auto' }}>
              CodeMaster Static Analyzer
            </span>
          </div>
        </div>
      )}
    </div>
  );
}
