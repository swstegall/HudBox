import React, {useState, useCallback} from 'react';
import clsx from 'clsx';
import Heading from '@theme/Heading';
import styles from './styles.module.css';

const FeatureList = [
    {
        title: 'Frameless GTK4 Window',
        Svg: require('@site/static/img/hudbox-window.svg').default,
        description: (
            <>
                A minimal, decoration‑less window ideal for HUDs, dashboards, and overlays on Linux desktops.
            </>
        ),
    },
    {
        title: 'WebKitGTK Inside',
        Svg: require('@site/static/img/hudbox-web.svg').default,
        description: (
            <>
                Load any web address you provide. Combine with transparency for stream overlays or status bars.
            </>
        ),
    },
    {
        title: 'Transparent & Draggable',
        Svg: require('@site/static/img/hudbox-transparency.svg').default,
        description: (
            <>
                Control opacity (0.0–1.0), enable full transparency, and click‑and‑drag content to move when unlocked.
            </>
        ),
    },
    {
        title: 'JSON Configuration',
        Svg: require('@site/static/img/hudbox-config.svg').default,
        description: (
            <>
                Configure one or many windows with a simple JSON file. A default config is auto‑generated
                at <code>~/.hudbox.json</code> if missing.
            </>
        ),
    },
];

function Feature({Svg, title, description, delayMs = 0}) {
    const [jello, setJello] = useState(false);

    const handleMouseEnter = useCallback(() => {
        // Trigger the jello animation by toggling the class
        setJello(true);
    }, []);

    const handleAnimationEnd = useCallback(() => {
        // Remove the class so it can be retriggered next hover
        setJello(false);
    }, []);

    return (
        <div
            className={clsx('col col--4', styles.feature)}
            onMouseEnter={handleMouseEnter}
        >
            <div className="text--center">
                <Svg
                    className={clsx(
                        styles.featureSvg,
                        'animate__animated',
                        jello && 'animate__jello',
                    )}
                    role="img"
                    onAnimationEnd={handleAnimationEnd}
                />
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
        <section className={clsx(styles.features)}>
            <div className="container">
                <div className="row">
                    {FeatureList.map((props, idx) => (
                        <Feature key={idx} {...props} delayMs={100 * idx}/>
                    ))}
                </div>
            </div>
        </section>
    );
}
