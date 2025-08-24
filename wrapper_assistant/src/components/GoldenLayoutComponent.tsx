
import React, { useEffect, useRef } from 'react';
import { GoldenLayout, LayoutConfig, ComponentContainer } from 'golden-layout';
import 'golden-layout/dist/css/goldenlayout-base.css';
import 'golden-layout/dist/css/themes/goldenlayout-light-theme.css';

export const GoldenLayoutComponent: React.FC = () => {
  const containerRef = useRef<HTMLDivElement>(null);
  const layoutRef = useRef<GoldenLayout | null>(null);

  useEffect(() => {
    if (containerRef.current && !layoutRef.current) {
      const config: LayoutConfig = {
        root: {
          type: 'row',
          content: [
            {
              type: 'component',
              componentType: 'sample',
              title: '示例窗口1',
            },
            {
              type: 'component',
              componentType: 'sample',
              title: '示例窗口2',
            },
          ],
        },
      };
      layoutRef.current = new GoldenLayout(config, containerRef.current);
      layoutRef.current.registerComponent('sample', (container: ComponentContainer) => {
        container.element!.innerHTML = '<div style="padding:16px;">这里是窗口内容</div>';
      });
      layoutRef.current.init();
    }
    return () => {
      if (layoutRef.current) {
        layoutRef.current.destroy();
        layoutRef.current = null;
      }
    };
  }, []);

  return <div ref={containerRef} style={{ height: '100%', width: '100%' }} />;
};
