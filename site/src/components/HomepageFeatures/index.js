import clsx from 'clsx';
import Heading from '@theme/Heading';
import styles from './styles.module.css';

const FeatureList = [
  {
    title: 'Frameless GTK4 Window',
    Svg: require('@site/static/img/undraw_docusaurus_mountain.svg').default,
    description: (
      <>
        A minimal, decoration‑less window ideal for HUDs, dashboards, and overlays on Linux desktops.
      </>
    ),
  },
  {
    title: 'WebKitGTK Inside',
    Svg: require('@site/static/img/undraw_docusaurus_tree.svg').default,
    description: (
      <>
        Load any web address you provide. Combine with transparency for stream overlays or status bars.
      </>
    ),
  },
  {
    title: 'Transparent & Draggable',
    Svg: require('@site/static/img/undraw_docusaurus_react.svg').default,
    description: (
      <>
        Control opacity (0.0–1.0), enable full transparency, and click‑and‑drag content to move when unlocked.
      </>
    ),
  },
  {
    title: 'JSON Configuration',
    Svg: require('@site/static/img/undraw_docusaurus_tree.svg').default,
    description: (
      <>
        Configure one or many windows with a simple JSON file. A default config is auto‑generated at <code>~/.hudbox.json</code> if missing.
      </>
    ),
  },
];

function Feature({Svg, title, description}) {
  return (
    <div className={clsx('col col--4')}>
      <div className="text--center">
        <Svg className={styles.featureSvg} role="img" />
      </div>
      <div className="text--center padding-horiz--md">
        <Heading as="h3">{title}</Heading>
        <p>{description}</p>
      </div>
    </div>
  );
}

export default function HomepageFeatures() {
  return (
    <section className={styles.features}>
      <div className="container">
        <div className="row">
          {FeatureList.map((props, idx) => (
            <Feature key={idx} {...props} />
          ))}
        </div>
      </div>
    </section>
  );
}
