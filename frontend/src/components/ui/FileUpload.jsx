import { useState, useRef } from 'react';
import { UploadCloud, FileType, CheckCircle, AlertCircle } from 'lucide-react';
import { analyzeCode } from '../../services/api';
import './FileUpload.css';

export default function FileUpload({ onAnalysisComplete }) {
  const [isDragging, setIsDragging] = useState(false);
  const [status, setStatus] = useState('idle'); // idle, loading, success, error
  const fileInputRef = useRef(null);

  const handleFile = async (file) => {
    if (!file) return;
    
    setStatus('loading');
    
    try {
      const text = await file.text();
      await analyzeCode(text);
      setStatus('success');
      if (onAnalysisComplete) {
        onAnalysisComplete();
      }
      setTimeout(() => setStatus('idle'), 3000);
    } catch (err) {
      console.error(err);
      setStatus('error');
      setTimeout(() => setStatus('idle'), 3000);
    }
  };

  const onDragOver = (e) => {
    e.preventDefault();
    setIsDragging(true);
  };

  const onDragLeave = () => {
    setIsDragging(false);
  };

  const onDrop = (e) => {
    e.preventDefault();
    setIsDragging(false);
    if (e.dataTransfer.files && e.dataTransfer.files.length > 0) {
      handleFile(e.dataTransfer.files[0]);
    }
  };

  return (
    <div 
      className={`ethereal-card file-upload ${isDragging ? 'glow-primary dragging' : ''}`}
      onDragOver={onDragOver}
      onDragLeave={onDragLeave}
      onDrop={onDrop}
      onClick={() => fileInputRef.current?.click()}
    >
      <input 
        type="file" 
        accept=".c,.cpp,.h" 
        ref={fileInputRef} 
        style={{ display: 'none' }} 
        onChange={(e) => handleFile(e.target.files[0])}
      />
      
      <div className="upload-content">
        {status === 'idle' && (
          <>
            <div className="upload-icon-wrapper">
              <UploadCloud size={32} className="cloud-icon" />
            </div>
            <h3>Upload Source File</h3>
            <p>Drag and drop your .c or .cpp file here, or click to browse.</p>
          </>
        )}
        
        {status === 'loading' && (
          <>
            <div className="upload-icon-wrapper pulse-anim">
              <FileType size={32} className="cloud-icon" />
            </div>
            <h3>Analyzing Code...</h3>
            <p>Running CodeMaster pipeline...</p>
          </>
        )}
        
        {status === 'success' && (
          <>
            <div className="upload-icon-wrapper success-anim">
              <CheckCircle size={32} style={{ color: 'var(--color-secondary)' }} />
            </div>
            <h3>Analysis Complete</h3>
            <p>Dashboard updated successfully.</p>
          </>
        )}

        {status === 'error' && (
          <>
            <div className="upload-icon-wrapper error-anim">
              <AlertCircle size={32} style={{ color: 'var(--color-error)' }} />
            </div>
            <h3>Analysis Failed</h3>
            <p>There was an error processing your file.</p>
          </>
        )}
      </div>
    </div>
  );
}
