import pandas as pd
import re

def process_bom(input_file, output_file):
    # 1. 读取CSV文件
    try:
        df = pd.read_csv(input_file)
        print(f"成功读取文件: {input_file}")
        print(f"原始数据行数: {len(df)}")
    except FileNotFoundError:
        print(f"错误：找不到文件 '{input_file}'")
        return
    except Exception as e:
        print(f"读取文件时出错: {e}")
        return

    # 2. 数据预处理
    # 将 Quantity 转为数值类型（防止字符串类型）
    df['Quantity'] = pd.to_numeric(df['Quantity'], errors='coerce').fillna(0)

    # 创建用于分组的列，处理空值
    # 将 NaN 填充为空字符串，这样两个 NaN 会被视为相同
    df['Comment_Normalized'] = df['Comment'].fillna('')
    df['Footprint_Normalized'] = df['Footprint'].fillna('')
    df['Value_Normalized'] = df['Value'].fillna('')

    # 3. 分组并聚合
    def _parse_designators(series):
        """把一组 Designator 字符串拆开、去重并做自然排序，然后用逗号连接返回。"""
        parts = []
        for s in series.dropna():
            for token in str(s).split(','):
                token = token.strip()
                if token:
                    parts.append(token)
        # 保持出现顺序去重
        parts = list(dict.fromkeys(parts))

        def _key(d):
            m = re.match(r"^([A-Za-z_+-]*)(\d+)$", d)
            if m:
                return (m.group(1), int(m.group(2)), d)
            return (d, 0, d)

        return ','.join(sorted(parts, key=_key))

    def _first_nonempty(series):
        for v in series.dropna():
            if str(v).strip() != '':
                return v
        return ''

    # 按归一化后的 Comment/Footprint/Value 分组（将空视为相等）并聚合
    grouped = df.groupby(['Comment_Normalized', 'Footprint_Normalized', 'Value_Normalized'], dropna=False)
    rows = []
    for _, g in grouped:
        designator = _parse_designators(g.get('Designator', pd.Series(dtype=object)))
        qty = g['Quantity'].sum()

        row = {
            'No.': _first_nonempty(g['No.']) if 'No.' in df.columns else '',
            'Quantity': int(qty) if pd.notna(qty) else 0,
            'Comment': _first_nonempty(g['Comment']),
            'Designator': designator,
            'Footprint': _first_nonempty(g['Footprint']),
            'Value': _first_nonempty(g['Value']),
            'Manufacturer Part': _first_nonempty(g['Manufacturer Part']) if 'Manufacturer Part' in df.columns else '',
            'Manufacturer': _first_nonempty(g['Manufacturer']) if 'Manufacturer' in df.columns else '',
            'Supplier Part': _first_nonempty(g['Supplier Part']) if 'Supplier Part' in df.columns else '',
            'Supplier': ', '.join([s for s in g.get('Supplier', pd.Series(dtype=object)).dropna().unique() if str(s).strip() != ''])
        }
        rows.append(row)

    merged_df = pd.DataFrame(rows)

    # 尽量保持原始列顺序（存在的列），其它列追加到末尾
    cols = [c for c in df.columns if c in merged_df.columns]
    remaining = [c for c in merged_df.columns if c not in cols]
    merged_df = merged_df[cols + remaining] if cols else merged_df[merged_df.columns]

    # 按 Quantity 降序，重置索引
    merged_df = merged_df.sort_values('Quantity', ascending=False).reset_index(drop=True)

    print(f"合并后数据行数: {len(merged_df)}")

    # 保存为CSV
    merged_df.to_csv(output_file, index=False, encoding='utf-8-sig')
    print(f"成功保存文件: {output_file}")

    return merged_df

if __name__ == "__main__":
    input_csv = 'bom_input.csv'  # 你的输入文件名
    output_csv = 'bom_merged.csv'  # 输出文件名
    
    process_bom(input_csv, output_csv)

